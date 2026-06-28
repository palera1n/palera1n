#include "run.h"

#include <stdlib.h>
#include <stdatomic.h>
#include <stdbool.h>
#include <pthread.h>
#include <inttypes.h>
#include <stdint.h>
#include <unistd.h>

#include "../utils.h"
#include "../globals.h"
#include "../paleinfo.h"
#include "../usb/shim.h"
#include "../usb/pongo_helper.h"

#include "checkm8.h"
#include "dfu.h"
#include "payload.h"

static void *exploit_thread(void *arg)
{
    LOG("Waiting for DFU devices...");
    shared_t *s = (shared_t *)arg;

    usb_handle_t handle;
    struct DeviceConfiguration deviceConfig;
    struct PayloadConfiguration payloadConfig;

    int stage = STAGE_PREPARE;
    bool ret = false;

    init_usb_handle(&handle, 0x5AC, 0x1227);

    while (stage != STAGE_JAILBREAK && !atomic_load(&s->stop)) {
        if (!wait_usb_handle(&handle)) {
            atomic_store(&s->result, -1);
            atomic_store(&s->stop, true);
            break;
        }

        atomic_store(&s->stage, stage);

        switch (stage) {
        case STAGE_PREPARE: {
            char *sn = get_usb_serial_number(&handle);
            if (!sn) goto fail;

            uint64_t cpid = dfu_serial_number_get_cpid(sn);

            if (!checkm8_find_device_configuration_for_cpid(cpid, &deviceConfig)) {
                free(sn);
                goto fail;
            }

            if (!checkm8_find_payload_configuration_for_cpid(cpid, &payloadConfig)) {
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
            stage = ret ? STAGE_SETUP : STAGE_PREPARE;
            break;
        case STAGE_SETUP:
            LOG("Setting up the device for exploitation");
            ret = checkm8_stage_setup(&handle, &deviceConfig);
            stage = ret ? STAGE_SPRAY : STAGE_PREPARE;
            break;
        case STAGE_SPRAY:
            LOG("Spraying the device with USB requests");
            ret = checkm8_stage_spray(&handle, &deviceConfig);
            stage = ret ? STAGE_PATCH : STAGE_PREPARE;
            break;
        case STAGE_PATCH:
            LOG("Right before trigger (bug setup)");
            ret = checkm8_stage_patch(&handle, &deviceConfig, &payloadConfig);
            if (!ret) goto fail;
            stage = STAGE_PONGO;
            break;
        case STAGE_PONGO:
            LOG("Booting pongoOS");

            close_usb_handle(&handle);
            sleep_ms(3000);

            checkm8_boot_pongo(&handle);

            atomic_store(&s->exploit_done, true);
            atomic_store(&s->pongo_done, true);
            atomic_store(&s->stage, STAGE_JAILBREAK);

            stage = STAGE_JAILBREAK;
            break;
        default:
            goto fail;
        }

        reset_usb_handle(&handle);
    }

    atomic_store(&s->result, 1);
    return NULL;

fail:
    atomic_store(&s->result, -1);
    atomic_store(&s->stop, true);
    return NULL;
}

static void *pongo_thread(void *arg)
{
    LOG("Waiting for Pongo devices...");
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
            seen = true;

            LOG_SUCCESS("Pongo device detected!");

            if (palerain_flags & palerain_option_pongo_exit) {
                close_usb_handle(&handle);
                atomic_store(&s->pongo_done, true);
                atomic_store(&s->stage, STAGE_DONE);
                break;
            }

            char paleinfo[64];
            snprintf(paleinfo, sizeof(paleinfo), "palera1n_flags 0x%" PRIx64, palerain_flags);

            char xargs_cmd[0x270];
            snprintf(xargs_cmd, sizeof(xargs_cmd), "xargs %s", boot_args);
            if ((palerain_flags & palerain_option_setup_rootful)) {
                strncat(xargs_cmd, " wdt=-1", 0x270 - strlen(xargs_cmd) - 1);
            }

            LOG("Boot arguments: %s", xargs_cmd);

            issue_pongo_command(&handle, "fuse lock");
            issue_pongo_command(&handle, "sep auto");
            upload_buffer_to_pongo(&handle, g_payload_kpf.data, g_payload_kpf.data_len);
            issue_pongo_command(&handle, "modload");
            issue_pongo_command(&handle, paleinfo);
            upload_buffer_to_pongo(&handle, g_payload_ramdisk.data, g_payload_ramdisk.data_len);
            issue_pongo_command(&handle, "ramdisk");
            upload_buffer_to_pongo(&handle, g_payload_overlay.data, g_payload_overlay.data_len);
            issue_pongo_command(&handle, "overlay");
            if (strlen(boot_args) > 0)
                issue_pongo_command(&handle, xargs_cmd);
            issue_pongo_command(&handle, "bootx");

            atomic_store(&s->pongo_done, true);
            atomic_store(&s->stage, STAGE_DONE);

            break;
        }
    }

    close_usb_handle(&handle);
    return NULL;
}

bool exploit(shared_t *state)
{
    pthread_t t1, t2;

    pthread_create(&t1, NULL, exploit_thread, state);
    pthread_create(&t2, NULL, pongo_thread, state);

    pthread_join(t1, NULL);
    pthread_join(t2, NULL);

    return atomic_load(&state->result) == 1 &&
           atomic_load(&state->exploit_done) &&
           atomic_load(&state->pongo_done);
}

#include "run.h"

#include <stdlib.h>
#include <stdatomic.h>
#include <stdbool.h>
#include <pthread.h>
#include <inttypes.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>

#include "../utils.h"
#include "../globals.h"
#include "../paleinfo.h"
#include "../usb/shim.h"
#include "../usb/pongo_helper.h"

#include "checkm8.h"
#include "dfu.h"
#include "payload.h"

#define VID_APPLE   (0x5AC)
#define PID_PONGO   (0x4141)
#define PID_DFU     (0x1227)

static void *exploit_thread(void *arg)
{
    LOG_VERBOSE("Starting thread for DFU devices");
    LOG("Waiting for DFU devices...");

    shared_t *s = (shared_t *)arg;

    usb_handle_t handle;
    struct DeviceConfiguration deviceConfig;
    struct PayloadConfiguration payloadConfig;

    int stage = STAGE_RESET;
    bool ret = false;

    init_usb_handle(&handle, VID_APPLE, PID_DFU);

    while (!atomic_load(&s->stop)) {
        if (!wait_usb_handle(&handle)) {
            reset_usb_handle(&handle);
            continue;
        }

        char *sn = get_usb_serial_number(&handle);
        if (!sn) {
            reset_usb_handle(&handle);
            continue;
        }

        uint64_t cpid = dfu_serial_number_get_cpid(sn);
        if (!checkm8_find_device_configuration_for_cpid(cpid, &deviceConfig)
            || !checkm8_find_payload_configuration_for_cpid(cpid, &payloadConfig))
        {
            free(sn);
            reset_usb_handle(&handle);
            continue;
        }

        if (dfu_serial_number_is_pwned(sn)) {
            LOG_WARN("Detected already PWN'd device, skipping");
            free(sn);
            reset_usb_handle(&handle);
            continue;
        } else if (dfu_serial_number_is_in_dfu_mode(sn)) {
            LOG_SUCCESS("Detected DFU device");
        } else if (dfu_serial_number_is_in_yolo_dfu(sn)) {
            LOG_SUCCESS("Detected YoloDFU device");
            stage = STAGE_YOLODFU;
        } else {
            free(sn);
            reset_usb_handle(&handle);
            continue;
        }

        free(sn);
        atomic_store(&s->stage, stage);

        switch (stage) {
            case STAGE_RESET:
                ret = checkm8_stage_reset(&handle);
                stage = ret ? STAGE_SETUP : STAGE_RESET;
                break;
            case STAGE_SETUP:
                LOG("Checking if device is ready");
                ret = checkm8_stage_setup(&handle, &deviceConfig);
                stage = ret ? STAGE_SPRAY : STAGE_RESET;
                break;
            case STAGE_SPRAY:
                LOG("Setting up the exploit (this is the heap spray)");
                ret = checkm8_stage_spray(&handle, &deviceConfig);
                stage = ret ? STAGE_PATCH : STAGE_RESET;
                break;
            case STAGE_PATCH:
                LOG("Right before trigger (this is the real bug setup)");
                ret = checkm8_stage_patch(&handle, &deviceConfig, &payloadConfig);
                stage = STAGE_RESET;
                if (!ret) {
                    LOG_ERROR("Failed doing trigger");
                    break;
                }
                continue;
            case STAGE_YOLODFU:
                LOG("Booting PongoOS...");
                ret = checkm8_boot_pongo(&handle);
                if (!ret) LOG_ERROR("Failed to send PongoOS to device");
                stage = STAGE_RESET;
                break;
        }

        reset_usb_handle(&handle);
    }

    close_usb_handle(&handle);
    return NULL;
}

static void *pongo_thread(void *arg)
{
    LOG_VERBOSE("Starting thread for PongoOS devices");

    shared_t *s = (shared_t *)arg;

    usb_handle_t handle;
    bool seen = false;

    init_usb_handle(&handle, VID_APPLE, PID_PONGO);

    while (!atomic_load(&s->stop)) {
        if (!wait_usb_handle(&handle)) {
            if (atomic_load(&s->stop)) break;
            reset_usb_handle(&handle);
            continue;
        }

        if (seen) continue;
        seen = true;

        LOG_SUCCESS("Detected PongoOS device");

        atomic_store(&s->stage, STAGE_PONGO);

        if (palerain_flags & palerain_option_pongo_exit) {
            close_usb_handle(&handle);
            atomic_store(&s->stage, STAGE_DONE);
            atomic_store(&s->stop, true);
            return NULL;
        }

        char paleinfo[64];
        snprintf(paleinfo, sizeof(paleinfo), "palera1n_flags 0x%" PRIx64, palerain_flags);

        char xargs_cmd[0x270];
        snprintf(xargs_cmd, sizeof(xargs_cmd), "xargs %s", boot_args);
        if (palerain_flags & palerain_option_setup_rootful) {
            strncat(xargs_cmd, " wdt=-1", sizeof(xargs_cmd) - strlen(xargs_cmd) - 1);
        }

        issue_pongo_command(&handle, "fuse lock");
        issue_pongo_command(&handle, "sep auto");
        if (g_payload_kpf.data_len > 0) {
            upload_buffer_to_pongo(&handle, g_payload_kpf.data, g_payload_kpf.data_len);
            issue_pongo_command(&handle, "modload");
        }
        issue_pongo_command(&handle, paleinfo);
        if (g_payload_ramdisk.data_len > 0) {
            upload_buffer_to_pongo(&handle, g_payload_ramdisk.data, g_payload_ramdisk.data_len);
            issue_pongo_command(&handle, "ramdisk");
        }
        if (g_payload_overlay.data_len > 0) {
            upload_buffer_to_pongo(&handle, g_payload_overlay.data, g_payload_overlay.data_len);
            issue_pongo_command(&handle, "overlay");
        }
        if (strlen(boot_args) > 0) issue_pongo_command(&handle, xargs_cmd);
        issue_pongo_command(&handle, "bootx");

        atomic_store(&s->stage, STAGE_DONE);
        atomic_store(&s->stop, true);
        break;
    }

    close_usb_handle(&handle);
    return NULL;
}

bool exploit(shared_t *state)
{
    pthread_t t1, t2;

    pthread_create(&t1, NULL, exploit_thread, state);
    pthread_create(&t2, NULL, pongo_thread, state);

    while (!atomic_load(&state->stop)) {
        sleep_ms(10);
    }

    pthread_cancel(t1);
    pthread_cancel(t2);

    pthread_join(t1, NULL);
    pthread_join(t2, NULL);

    return atomic_load(&state->result) == 1;
}

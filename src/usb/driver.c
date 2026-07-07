#ifdef _WIN32

#include "driver.h"

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <setupapi.h>
#include <devguid.h>
#pragma comment(lib, "setupapi.lib")

#include <libwdi.h>

#include "../utils.h"

#define MAX_MATCHING_DRIVERS 16

static void add_target_driver(char list[MAX_MATCHING_DRIVERS][64], int *count, const char *inf)
{
    if (*count >= MAX_MATCHING_DRIVERS || strlen(inf) == 0) return;
    
    for (int i = 0; i < *count; i++) {
        if (strcmp(list[i], inf) == 0) return;
    }
    
    strncpy(list[*count], inf, 63);
    list[*count][63] = '\0';
    (*count)++;
}

driver_result_t install_libusbk_target(unsigned short vid, unsigned short pid)
{
    struct wdi_device_info *list = NULL;
    struct wdi_options_create_list cl = { .list_all = TRUE };
    struct wdi_options_prepare_driver prep = {
        .driver_type = WDI_LIBUSBK,
        .vendor_name = "libusbK"
    };
    struct wdi_options_install_driver inst = {0};

    LOG_VERBOSE("Target: VID:%04X PID:%04X", vid, pid);

    if (wdi_create_list(&list, &cl) != WDI_SUCCESS) {
        LOG_ERROR("wdi_create_list failed");
        return DRIVER_ERROR;
    }

    char temp_path[MAX_PATH];
    char driver_path[MAX_PATH];

    if (GetTempPathA(MAX_PATH, temp_path) == 0) {
        LOG_ERROR("GetTempPathA failed");
        wdi_destroy_list(list);
        return DRIVER_ERROR;
    }

    snprintf(driver_path, sizeof(driver_path), "%slibusbk_driver\\", temp_path);

    CreateDirectoryA(driver_path, NULL);

    bool found = false;
    bool installed = false;
    bool failed = false;

    for (struct wdi_device_info *d = list; d; d = d->next) {
        if (d->vid != vid || d->pid != pid)
            continue;

        found = true;

        if (d->driver && _stricmp(d->driver, "libusbK") == 0) {
            LOG_VERBOSE(
                "Device %04X:%04X already using libusbK",
                vid,
                pid
            );
            installed = true;
            break;
        }

        LOG("Installing libusbK for %04X:%04X...", vid, pid);

        if (wdi_prepare_driver(
                d,
                driver_path,
                "libusbk_driver.inf",
                &prep
            ) != WDI_SUCCESS)
        {
            LOG_ERROR("wdi_prepare_driver failed");
            failed = true;
            continue;
        }

        if (wdi_install_driver(
                d,
                driver_path,
                "libusbk_driver.inf",
                &inst
            ) != WDI_SUCCESS)
        {
            LOG_ERROR("wdi_install_driver failed");
            failed = true;
            continue;
        }

        LOG_SUCCESS(
            "libusbK driver successfully installed for %04X:%04X",
            vid,
            pid
        );

        installed = true;
    }

    wdi_destroy_list(list);

    if (!found)
        return DRIVER_NOT_PRESENT;

    if (installed && !failed)
        return DRIVER_SUCCESS;

    return DRIVER_ERROR;
}

driver_result_t uninstall_libusbk_target(unsigned short vid, unsigned short pid)
{
    char hwid_u[64], hwid_l[64];
    snprintf(hwid_u, sizeof(hwid_u), "USB\\VID_%04X&PID_%04X", vid, pid);
    snprintf(hwid_l, sizeof(hwid_l), "usb\\vid_%04x&pid_%04x", vid, pid);

    LOG_VERBOSE("Target Signature: %s (libusbK)", hwid_u);

    char temp_path[MAX_PATH], log_path[MAX_PATH], cmd[512];
    if (GetTempPathA(MAX_PATH, temp_path) == 0) return DRIVER_ERROR;
    snprintf(log_path, sizeof(log_path), "%spnputil_drivers.txt", temp_path);

    // Strip active instances first
    snprintf(cmd, sizeof(cmd), "pnputil /remove-device /deviceid \"%s\" >nul 2>&1", hwid_u);
    system(cmd);

    // Dump staging tables
    snprintf(cmd, sizeof(cmd), "pnputil /enum-drivers > \"%s\"", log_path);
    system(cmd);

    FILE *fp = fopen(log_path, "r");
    char line[256], current_oem_inf[64] = {0};
    char target_drivers_list[MAX_MATCHING_DRIVERS][64];
    int target_count = 0, is_libusbk = 0, matches_hwid = 0;

    if (fp) {
        while (fgets(line, sizeof(line), fp)) {
            if (strstr(line, "Published Name:") || strstr(line, "Published name:")) {
                if (is_libusbk && matches_hwid) {
                    add_target_driver(target_drivers_list, &target_count, current_oem_inf);
                }
                is_libusbk = matches_hwid = 0;
                current_oem_inf[0] = '\0';

                char *inf_ptr = strstr(line, "oem");
                if (inf_ptr) sscanf(inf_ptr, "%63s", current_oem_inf);
            }

            if (strstr(line, "libusbK") || strstr(line, "libusbk")) is_libusbk = 1;
            if (strstr(line, hwid_u) || strstr(line, hwid_l) || strstr(line, "libusbk_driver.inf")) matches_hwid = 1;
        }
        if (is_libusbk && matches_hwid) {
            add_target_driver(target_drivers_list, &target_count, current_oem_inf);
        }
        fclose(fp);
        DeleteFileA(log_path);
    }

    if (target_count > 0) {
        for (int i = 0; i < target_count; i++) {
            LOG("Purging driver package (%d/%d): %s", i + 1, target_count, target_drivers_list[i]);
            snprintf(cmd, sizeof(cmd), "pnputil /delete-driver %s /uninstall /force >nul 2>&1", target_drivers_list[i]);
            system(cmd);
        }
        snprintf(cmd, sizeof(cmd), "pnputil /scan-devices >nul 2>&1");
        system(cmd);
        LOG_SUCCESS("All %d libusbK driver instances fully uninstalled.", target_count);
        return DRIVER_SUCCESS;
    }

    LOG_ERROR("No matching libusbK driver package found in system store");
    return DRIVER_NOT_PRESENT;
}

bool usb_device_present(unsigned short vid, unsigned short pid)
{
    HDEVINFO info;
    SP_DEVINFO_DATA dev_info;

    char target[64];

    snprintf(
        target,
        sizeof(target),
        "VID_%04X&PID_%04X",
        vid,
        pid
    );

    info = SetupDiGetClassDevsA(
        NULL,
        "USB",
        NULL,
        DIGCF_PRESENT | DIGCF_ALLCLASSES
    );

    if (info == INVALID_HANDLE_VALUE)
        return false;

    dev_info.cbSize = sizeof(dev_info);

    for (DWORD i = 0;
         SetupDiEnumDeviceInfo(info, i, &dev_info);
         i++)
    {
        char hardware_ids[4096];
        DWORD size = 0;

        if (!SetupDiGetDeviceRegistryPropertyA(
                info,
                &dev_info,
                SPDRP_HARDWAREID,
                NULL,
                (PBYTE)hardware_ids,
                sizeof(hardware_ids),
                &size))
        {
            continue;
        }

        char *id = hardware_ids;

        while (*id) {
            if (strstr(id, target)) {
                SetupDiDestroyDeviceInfoList(info);
                return true;
            }

            id += strlen(id) + 1;
        }
    }

    SetupDiDestroyDeviceInfoList(info);

    return false;
}

#endif
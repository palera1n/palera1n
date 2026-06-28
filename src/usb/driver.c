#ifdef _WIN32

#include "driver.h"

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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

int install_libusbk_target(unsigned short vid, unsigned short pid)
{
    struct wdi_device_info *list = NULL;
    struct wdi_options_create_list cl = { .list_all = TRUE };
    struct wdi_options_prepare_driver prep = { .driver_type = WDI_LIBUSBK, .vendor_name = "libusbK" };
    struct wdi_options_install_driver inst = {0};

    LOG_VERBOSE("Target: VID:%04X PID:%04X", vid, pid);

    if (wdi_create_list(&list, &cl) != WDI_SUCCESS) {
        LOG_ERROR("wdi_create_list failed\n");
        return 1;
    }

    char temp_path[MAX_PATH], driver_path[MAX_PATH];
    if (GetTempPathA(MAX_PATH, temp_path) == 0) {
        LOG_ERROR("GetTempPathA failed\n");
        wdi_destroy_list(list);
        return 1;
    }

    snprintf(driver_path, sizeof(driver_path), "%slibusbk_driver\\", temp_path);
    CreateDirectoryA(driver_path, NULL);

    int found = 0;
    for (struct wdi_device_info *d = list; d; d = d->next) {
        if (d->vid != vid || d->pid != pid) continue;

        found = 1;
        LOG("Found device %04X:%04X. Preparing & installing driver...", vid, pid);

        if (wdi_prepare_driver(d, driver_path, "libusbk_driver.inf", &prep) != WDI_SUCCESS ||
            wdi_install_driver(d, driver_path, "libusbk_driver.inf", &inst) != WDI_SUCCESS) {
            LOG_ERROR("Driver management pipeline failed for instance node");
        } else {
            LOG_SUCCESS("Driver successfully installed");
        }
    }

    wdi_destroy_list(list);
    return found ? 0 : 2;
}

int uninstall_libusbk_target(unsigned short vid, unsigned short pid)
{
    char hwid_u[64], hwid_l[64];
    snprintf(hwid_u, sizeof(hwid_u), "USB\\VID_%04X&PID_%04X", vid, pid);
    snprintf(hwid_l, sizeof(hwid_l), "usb\\vid_%04x&pid_%04x", vid, pid);

    LOG_VERBOSE("Target Signature: %s (libusbK)", hwid_u);

    char temp_path[MAX_PATH], log_path[MAX_PATH], cmd[512];
    if (GetTempPathA(MAX_PATH, temp_path) == 0) return 1;
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
        return 0;
    }

    LOG_ERROR("No matching libusbK driver package found in system store");
    return 2;
}

bool is_installed_with_libusbk(unsigned short vid, unsigned short pid)
{
    struct wdi_device_info *list = NULL;
    struct wdi_options_create_list cl = { .list_all = TRUE }; 
    bool has_libusbk = false;

    LOG_VERBOSE("Checking driver allocation for VID:%04X PID:%04X", vid, pid);

    if (wdi_create_list(&list, &cl) != WDI_SUCCESS) {
        LOG_ERROR("wdi_create_list failed during driver status check");
        return false;
    }

    for (struct wdi_device_info *d = list; d; d = d->next) {
        if (d->vid == vid && d->pid == pid) {
            if (d->driver && (stricmp(d->driver, "libusbK") == 0)) {
                has_libusbk = true;
                break;
            }
        }
    }

    wdi_destroy_list(list);
    return has_libusbk;
}

#endif
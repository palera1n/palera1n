#if defined(WITH_GUI) || defined(WITH_TUI)

#include "device_info.hpp"

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include <cstdio>
#include <cstdint>
#include <cstdlib>
#include <cstring>

static uint64_t get_hex_field(const char *serial, const char *field) {
    char *p = strstr((char *)serial, field);
    return p ? strtoull(p + strlen(field), nullptr, 16) : 0;
}

void get_recovery_info(char *sn, const char **ecid, const char **product_model, const char **product_name)
{
    static char cpid_buf[16];
    static char ecid_buf[32];
    static char bdid_buf[16];

    uint16_t cpid_val = (uint16_t)get_hex_field(sn, "CPID:");
    uint32_t bdid_val = (uint32_t)get_hex_field(sn, "BDID:");
    uint64_t ecid_val = get_hex_field(sn, "ECID:");

    snprintf(cpid_buf, sizeof(cpid_buf), "%04X", cpid_val);
    snprintf(bdid_buf, sizeof(bdid_buf), "%08X", bdid_val);
    snprintf(ecid_buf, sizeof(ecid_buf), "%016llX", (unsigned long long)ecid_val);

    *ecid = ecid_buf;

    *product_model = nullptr;
    *product_name = nullptr;

    for (int i = 0; irecv_devices[i].product_type; i++) {
        if (irecv_devices[i].cpid == cpid_val &&
            irecv_devices[i].bdid == bdid_val) {
            *product_model = irecv_devices[i].product_type;
            *product_name = irecv_devices[i].name;
            break;
        }
    }
}

void get_name_from_product_type(const char *product_type, const char **name) {
    *name = nullptr;
    for (int i = 0; irecv_devices[i].product_type; i++) {
        if (strcmp(irecv_devices[i].product_type, product_type) == 0) {
            *name = irecv_devices[i].name;
            break;
        }
    }
}

#endif // WITH_GUI || WITH_TUI

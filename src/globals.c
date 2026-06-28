#include "globals.h"

#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stddef.h>
#include <string.h>

#include "gen/payloads/binpack.h"
#include "gen/payloads/ramdisk.h"
#include "gen/payloads/Pongo.h"
#include "gen/payloads/checkra1n-kpf-pongo.h"

uint64_t palerain_flags = 0;
char boot_args[0x270] = { '\0' };

payload_t g_payload_overlay = {
    .data = payloads_binpack_dmg,
    .data_len = payloads_binpack_dmg_len,
};

payload_t g_payload_ramdisk = {
    .data = payloads_ramdisk_dmg,
    .data_len = payloads_ramdisk_dmg_len,
};

payload_t g_payload_pongo = {
    .data = payloads_Pongo_bin,
    .data_len = payloads_Pongo_bin_len,
};

payload_t g_payload_kpf = {
    .data = payloads_checkra1n_kpf_pongo,
    .data_len = payloads_checkra1n_kpf_pongo_len,
};

bool override_payload_from_file(const char *path, payload_t *out)
{
    FILE *f = fopen(path, "rb");
    if (!f) return false;

    if (fseek(f, 0, SEEK_END) != 0) {
        fclose(f);
        return false;
    }

    long len = ftell(f);
    if (len <= 0) {
        fclose(f);
        return false;
    }
    rewind(f);

    void *buf = malloc((size_t)len);
    if (!buf) {
        fclose(f);
        return false;
    }

    if (fread(buf, 1, (size_t)len, f) != (size_t)len) {
        free(buf);
        fclose(f);
        return false;
    }

    fclose(f);

    out->data = buf;
    out->data_len = (size_t)len;

    return true;
}

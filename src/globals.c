#include "globals.h"

#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stddef.h>
#include <string.h>

#if WITH_BINPACK
# include "gen/embedded/binpack.h"
#endif
#if WITH_RAMDISK
# include "gen/embedded/ramdisk.h"
# include "gen/embedded/ramdisk-compressed.dmg.h"
#endif
#include "gen/embedded/Pongo.h"
#include "gen/embedded/checkra1n-kpf-pongo.h"
#include "gen/embedded/checkra1n-kpf-pongo-compressed.h"

uint64_t palerain_flags = 0;
char boot_args[0x270] = { '\0' };

#if WITH_BINPACK
payload_t g_payload_overlay = {
    .data = embedded_binpack_dmg,
    .data_len = embedded_binpack_dmg_len,
    .uncompressed_data_len = 0,
};
#else
payload_t g_payload_overlay = {
    .data = NULL,
    .data_len = 0,
    .uncompressed_data_len = 0,
};
#endif

#if WITH_RAMDISK
payload_t g_payload_ramdisk = {
    .data = embedded_ramdisk_compressed_dmg_lzma,
    .data_len = embedded_ramdisk_compressed_dmg_lzma_len,
    .uncompressed_data_len = embedded_ramdisk_dmg_len,
};
#else
payload_t g_payload_ramdisk = {
    .data = NULL,
    .data_len = 0,
    .uncompressed_data_len = 0,
};
#endif

payload_t g_payload_pongo = {
    .data = embedded_Pongo_bin,
    .data_len = embedded_Pongo_bin_len,
    .uncompressed_data_len = 0,
};

payload_t g_payload_kpf = {
    .data = embedded_checkra1n_kpf_pongo_compressed_lzma,
    .data_len = embedded_checkra1n_kpf_pongo_compressed_lzma_len,
    .uncompressed_data_len = embedded_checkra1n_kpf_pongo_len,
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
    // assume uncompressed
    out->uncompressed_data_len = 0;

    return true;
}

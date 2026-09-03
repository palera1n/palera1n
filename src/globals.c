/*
 * palera1n - https://palera.in
 *
 * Copyright (C) 2026 palera1n team
 *
 * SPDX-License-Identifier: MIT
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 */

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
# include "gen/embedded/ramdisk-compressed.h"
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
    .data = embedded_ramdisk_compressed_lzma,
    .data_len = embedded_ramdisk_compressed_lzma_len,
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

#define PONGO_MAX_SZ    (0x7fe00)
#define MACHO_MAGIC_32  (0xFEEDFACE)
#define MACHO_MAGIC_64  (0xFEEDFACF)

bool check_payload_validity(const payload_t *payload, bool isPongo)
{
    if (!payload || !payload->data || payload->data_len < 4)
        return false;

    const uint32_t magic = *(const uint32_t *)payload->data;
    const bool isMachO = (magic == MACHO_MAGIC_32 || magic == MACHO_MAGIC_64);

    if (isPongo) {
        if (isMachO || payload->data_len > PONGO_MAX_SZ)
            return false;
    } else {
        if (!isMachO)
            return false;
    }

    return true;
}

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
    // assume overwritten file is uncompressed
    // this ultimately determines if pongo should
    // decompress these files. user provided
    // files should not be compressed.
    out->uncompressed_data_len = 0;

    return true;
}

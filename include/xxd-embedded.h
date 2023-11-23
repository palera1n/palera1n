#ifndef XXD_EMBEDDED_H
#define XXD_EMBEDDED_H

#if defined(__APPLE__)
#include <TargetConditionals.h>
#endif

#include <stdint.h>

extern uint8_t checkra1n[];
extern uint32_t checkra1n_len;

extern uint8_t checkra1n_kpf_pongo_lzma[];
extern uint32_t checkra1n_kpf_pongo_lzma_len;

extern uint8_t ramdisk_dmg_lzma[];
extern uint32_t ramdisk_dmg_lzma_len;

extern uint8_t binpack_dmg[];
extern uint32_t binpack_dmg_len;

extern uint8_t Pongo_bin[];
extern uint32_t Pongo_bin_len;

#if defined(__APPLE__) && (TARGET_OS_IPHONE || defined(DEV_BUILD) || defined(FORCE_HELPER))
extern uint8_t libcheckra1nhelper_dylib[];
extern uint32_t libcheckra1nhelper_dylib_len;
#endif

#endif

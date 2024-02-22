#include <stdlib.h>

#ifdef NO_KPF
unsigned char* checkra1n_kpf_pongo_lzma = NULL;
unsigned int checkra1n_kpf_pongo_lzma_len = 0;
#endif

#ifdef NO_RAMDISK
unsigned char* ramdisk_dmg_lzma = NULL;
unsigned int ramdisk_dmg_lzma_len = 0;
#endif

#ifdef NO_CHECKRAIN
unsigned char* checkra1n = NULL;
unsigned int checkra1n_len = 0;
#endif

#ifdef NO_OVERLAY
unsigned char* binpack_dmg = NULL;
unsigned int binpack_dmg_len = 0;
#endif

#ifdef NO_CUSTOM_PONGO
unsigned char* Pongo_bin = NULL;
unsigned int Pongo_bin_len = 0;
#endif

#ifdef NO_EMBED_HELPER
unsigned char* libcheckra1nhelper_dylib = NULL;
unsigned int libcheckra1nhelper_dylib_len = 0;
#endif

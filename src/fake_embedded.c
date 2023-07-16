#include <stdlib.h>

#ifdef NO_KPF
unsigned char* checkra1n_kpf_pongo = NULL;
unsigned int checkra1n_kpf_pongo_len = 0;
#endif

#ifdef NO_RAMDISK
unsigned char* ramdisk_dmg = NULL;
unsigned int ramdisk_dmg_len = 0;
#endif

#ifdef NO_OVERLAY
unsigned char* binpack_dmg = NULL;
unsigned int binpack_dmg_len = 0;
#endif

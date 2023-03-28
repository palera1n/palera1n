#ifndef XXD_EMBEDDED_H
#define XXD_EMBEDDED_H

#if defined(__APPLE__)
#include <TargetConditionals.h>
#endif

#ifndef NO_CHECKRAIN
extern unsigned char checkra1n[];
extern unsigned int checkra1n_len;
#endif

#ifndef NO_KPF
extern unsigned char checkra1n_kpf_pongo[];
extern unsigned int checkra1n_kpf_pongo_len;
#endif

#ifndef NO_RAMDISK
extern unsigned char ramdisk_dmg[];
extern unsigned int ramdisk_dmg_len;
#endif

#ifndef NO_BINPACK
extern unsigned char binpack_dmg[];
extern unsigned int binpack_dmg_len;
#endif

#ifndef NO_CUSTOM_PONGO
extern unsigned char Pongo_bin[];
extern unsigned int Pongo_bin_len;
#endif

#if defined(__APPLE__) && (TARGET_OS_IPHONE || defined(DEV_BUILD))
extern unsigned char libcheckra1nhelper_dylib[];
extern unsigned int libcheckra1nhelper_dylib_len;
#endif

#endif

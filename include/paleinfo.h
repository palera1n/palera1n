/*
 * jbinit - https://github.com/palera1n/jbinit
 *
 * This file is part of jbinit
 * 
 * SPDX-License-Identifier: MIT
 */

#ifndef PALEINFO_H
#define PALEINFO_H

#include <stdint.h>

#define PALEINFO_VERSION 2U
typedef uint64_t palerain_option_t;

// unified palera1n options
#define palerain_option_rootful              (UINT64_C(1) << 0) /* rootful jailbreak */
#define palerain_option_rootless             (UINT64_C(1) << 1) /* rootless jailbreak */
#define palerain_option_setup_rootful        (UINT64_C(1) << 2) /* create fakefs */
/* reserved */
#define palerain_option_setup_partial_root   (UINT64_C(1) << 4) /* fakefs creating should be partial */
#define palerain_option_checkrain_is_clone   (UINT64_C(1) << 5) /* supplied checkra1n is checkra1n clone */
#define palerain_option_rootless_livefs      (UINT64_C(1) << 6) /* mount root livefs on rootless */
// #define palerain_option_no_ssv            (UINT64_C(1) << 7) /* no signed system volume */
// #define palerain_option_force_fakefs      (UINT64_C(1) << 8) /* force fakefs, even without SSV */
#define palerain_option_clean_fakefs         (UINT64_C(1) << 9) /* clean fakefs, but does not delete it */
#define palerain_option_tui                  (UINT64_C(1) << 10) /* terminal user interface */
// #define palerain_option_gui               (UINT64_C(1) << 11) /* graphical user interface*/
#define palerain_option_dfuhelper_only       (UINT64_C(1) << 12) /* dfuhelper only */
#define palerain_option_pongo_exit           (UINT64_C(1) << 13) /* boot to clean pongo shell */
#define palerain_option_demote               (UINT64_C(1) << 14) /* Demote effective production fuse status */
#define palerain_option_pongo_full           (UINT64_C(1) << 15) /* Boot to pongo with default images and options */
#define palerain_option_palerain_version     (UINT64_C(1) << 16) /* Print version */
#define palerain_option_exit_recovery        (UINT64_C(1) << 17) /* Exit recovery mode */
#define palerain_option_reboot_device        (UINT64_C(1) << 18) /* Reboot device in normal mode */
#define palerain_option_enter_recovery       (UINT64_C(1) << 19) /* Enter recovery mode */
#define palerain_option_device_info          (UINT64_C(1) << 20) /* Print device info */
#define palerain_option_no_colors            (UINT64_C(1) << 21) /* no colors */
#define palerain_option_bind_mount           (UINT64_C(1) << 22) /* bind mounts should be used (always true iOS 15+)*/
#define palerain_option_overlay              (UINT64_C(1) << 23) /* there is an overlay (should always be true) */
#define palerain_option_force_revert         (UINT64_C(1) << 24) /* Unjailbreak */
#define palerain_option_safemode             (UINT64_C(1) << 25) /* Enter safe mode */
#define palerain_option_verbose_boot         (UINT64_C(1) << 26) /* verbose boot */

#define palerain_option_jbinit_log_to_file   (UINT64_C(1) << 50) /* make ramdisk log to file (jbinit2) */
#define palerain_option_setup_rootful_forced (UINT64_C(1) << 51) /* create fakefs over an existing one (jbinit2) */

#define palerain_option_flower_chain         (UINT64_C(1) << 61) /* Flower chain */
#define palerain_option_test1                (UINT64_C(1) << 62) /* Developer test option 1 */
#define palerain_option_test2                (UINT64_C(1) << 63) /* Developer test option 2 */

#define PALEINFO_MAGIC 'PLSH'

/* paleinfo version 2, appended to ramdisk */
struct paleinfo {
    uint32_t magic; /* 'PLSH' */
    uint32_t version; /* 2 */
    uint64_t kbase; /* kernel base */
    uint64_t kslide; /* kernel slide */
    uint64_t flags; /* unified palera1n flags */
    char rootdev[0x10]; /* ex. disk0s1s8 */
}__attribute__((packed));

#endif

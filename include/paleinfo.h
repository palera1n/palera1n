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
#define palerain_option_rootful              (1ULL << 0) /* rootful jailbreak */
#define palerain_option_rootless             (1ULL << 1) /* rootless jailbreak */
#define palerain_option_setup_rootful        (1ULL << 2) /* create fakefs */
/* reserved */
#define palerain_option_setup_partial_root   (1ULL << 4) /* fakefs creating should be partial */
#define palerain_option_checkrain_is_clone   (1ULL << 5) /* supplied checkra1n is checkra1n clone */
#define palerain_option_rootless_livefs      (1ULL << 6) /* mount root livefs on rootless */
// #define palerain_option_no_ssv            (1ULL << 7) /* no signed system volume */
// #define palerain_option_force_fakefs      (1ULL << 8) /* force fakefs, even without SSV */
#define palerain_option_clean_fakefs         (1ULL << 9) /* clean fakefs, but does not delete it */
#define palerain_option_tui                  (1ULL << 10) /* terminal user interface */
// #define palerain_option_gui               (1ULL << 11) /* graphical user interface*/
#define palerain_option_dfuhelper_only       (1ULL << 12) /* dfuhelper only */
#define palerain_option_pongo_exit           (1ULL << 13) /* boot to clean pongo shell */
#define palerain_option_demote               (1ULL << 14) /* Demote effective production fuse status */
#define palerain_option_pongo_full           (1ULL << 15) /* Boot to pongo with default images and options */
#define palerain_option_palerain_version     (1ULL << 16) /* Print version */
#define palerain_option_exit_recovery        (1ULL << 17) /* Exit recovery mode */
#define palerain_option_reboot_device        (1ULL << 18) /* Reboot device in normal mode */
#define palerain_option_enter_recovery       (1ULL << 19) /* Enter recovery mode */
#define palerain_option_device_info          (1ULL << 20) /* Print device info */
#define palerain_option_no_colors            (1ULL << 21) /* no colors */
#define palerain_option_bind_mount           (1ULL << 22) /* bind mounts should be used (always true iOS 15+)*/
#define palerain_option_overlay              (1ULL << 23) /* there is an overlay (should always be true) */
#define palerain_option_force_revert         (1ULL << 24) /* Unjailbreak */
#define palerain_option_safemode             (1ULL << 25) /* Enter safe mode */
#define palerain_option_verbose_boot         (1ULL << 26) /* verbose boot */

#define palerain_option_jbinit_log_to_file   (1ULL << 50) /* make ramdisk log to file (jbinit2) */
#define palerain_option_setup_rootful_forced (1ULL << 51) /* create fakefs over an existing one (jbinit2) */

#define palerain_option_flower_chain         (1ULL << 61) /* Flower chain */
#define palerain_option_test1                (1ULL << 62) /* Developer test option 1 */
#define palerain_option_test2                (1ULL << 63) /* Developer test option 2 */

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

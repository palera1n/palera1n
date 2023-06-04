/*
 * pongoOS - https://checkra.in
 *
 * Copyright (C) 2019-2022 checkra1n team
 *
 * This file is part of pongoOS.
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
#ifndef _KERNINFO_H
#define _KERNINFO_H

#include <stdbool.h>
#include <stdint.h>

#define MAX_BOOTARGS_LEN 256

#define checkrain_option_none               0x00000000
#define checkrain_option_all                0x7fffffff
#define checkrain_option_failure            0x80000000

// Host options
#define checkrain_option_verbose_logging    (1 << 0)
#define checkrain_option_demote             (1 << 1)
#define checkrain_option_early_exit         (1 << 2)
#define checkrain_option_quick_mode         (1 << 3)
#define checkrain_option_pongo_shell        (1 << 4)
#define checkrain_option_pongo_full         (1 << 5)

// KPF options
#define checkrain_option_verbose_boot       (1 << 0)

// Global options
#define checkrain_option_safemode           (1 << 0)
#define checkrain_option_bind_mount         (1 << 1)
#define checkrain_option_overlay            (1 << 2)
#define checkrain_option_force_revert       (1 << 7) /* keep this at 7 */

// palera1n option
#define palerain_option_rootful              (1 << 0) /* rootful jailbreak */
#define palerain_option_jbinit_log_to_file   (1 << 1) /* log to /cores/jbinit.log */
#define palerain_option_setup_rootful        (1 << 2) /* create fakefs */
#define palerain_option_setup_rootful_forced (1 << 3) /* create fakefs over an existing one */
#define palerain_option_setup_partial_root   (1 << 4) /* fakefs creating should be partial */
#define palerain_option_checkrain_is_clone   (1 << 5) /* supplied checkra1n is checkra1n clone */
#define palerain_option_rootless_livefs      (1 << 6) /* mount root livefs on rootless */
/* reserved values */
// #define palerain_option_no_ssv               (1 << 7) /* no signed system volume */
// #define palerain_option_force_fakefs         (1 << 8) /* force fakefs, even without SSV */
// #define palerain_option_rootless             (1 << 9) /* rootless jailbreak */
#define palerain_option_clean_fakefs        (1 << 10) /* clean fakefs, but does not delete it */

#ifdef DEV_BUILD
#define palerain_option_test1               (1 << 29)
#define palerain_option_test2               (1 << 30)
#endif

// palerain host options
#define host_option_dfuhelper_only       (1 << 0) /* dfuhelper only */
#define host_option_pongo_exit           (1 << 1) /* boot to clean pongo shell */
#ifdef TUI
#define host_option_tui                  (1 << 2) /* use terminal user interface */
#else
#define host_option_res1                 (1 << 2) /* reserved */
#endif
#define host_option_demote               (1 << 3) /* Demote effective production fuse status */
// (1 << 4) is removed
#define host_option_pongo_full           (1 << 5) /* Boot to pongo with default images and options */
#define host_option_palerain_version     (1 << 6) /* Print version */
#define host_option_exit_recovery        (1 << 7) /* Exit recovery mode */
#define host_option_reboot_device        (1 << 8) /* Reboot device in normal mode */
#define host_option_enter_recovery       (1 << 9) /* Enter recovery mode */
#define host_option_device_info          (1 << 10) /* Print device info */
#define host_option_no_colors            (1 << 11) /* no colors */

#define PALEINFO_MAGIC 'PLSH'
#define PALEINFO_CIGAM 'HSLP'

typedef uint32_t checkrain_option_t, *checkrain_option_p;

typedef enum {
    jailbreak_capability_tfp0               = 1 << 0,
    jailbreak_capability_userspace_reboot   = 1 << 1,
    jailbreak_capability_dyld_ignore_os     = 1 << 2, // TODO: This needs a better name
} jailbreak_capability_t, *jailbreak_capability_p;

#define DEFAULT_CAPABILITIES (jailbreak_capability_tfp0|jailbreak_capability_userspace_reboot)
struct kerninfo {
    uint64_t size;
    uint64_t base;
    uint64_t slide;
    checkrain_option_t flags;
};
struct paleinfo {
    uint32_t magic; // 'PLSH' / 0x504c5348
    uint32_t version; // 1
    checkrain_option_t flags;
    char rootdev[0x10];
};
struct kpfinfo {
    struct kerninfo k;
    checkrain_option_t kpf_flags;
    char bootargs[MAX_BOOTARGS_LEN];
};

#define checkrain_set_option(options, option, enabled) do { \
    if (enabled)                                            \
        options = (checkrain_option_t)(options | option);   \
    else                                                    \
        options = (checkrain_option_t)(options & ~option);  \
} while (0);

static inline bool checkrain_options_enabled(checkrain_option_t flags, checkrain_option_t opt)
{
    return (flags & opt) != 0;
}

#endif

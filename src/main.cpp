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

#include <unistd.h>
#include <cstdlib>
#include <iostream>
#include <getopt.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <inttypes.h>  // PRIx64

#ifdef WITH_GUI
# include <wx/wx.h>
#endif

#include "exploit.h"
#ifdef WITH_TUI
# include "tui/Tui.hpp"
#endif
#if WITH_CIDERRAIN
# include <ciderra1n/log.h>
extern "C" {
# include <ciderra1n/ra1n.h>
}
#else
# include <openra1n/utils.h>
#endif
#include "globals.h"
#include "paleinfo.h"
#include "pongo_helper.h"

// MARK: print_credits

void print_credits() {
    printf(
        "::\n"
        ":: Palera1n " PALERAIN_VERSION "\n"
        "::\n"
        ":: Copyright (C) 2026 palera1n team\n"
        "::\n"
        ":: ========  Made by  =======>\n"
        ":: Made by: asdfugil, kok3shidoll, claration, mineek\n"
        ":: plooshi, staturnz\n"
        ":: ======== Thanks to =======>\n"
        ":: Thanks to: itsnebulalol, llsc12, lrdsnow, dedbeddedbed\n"
        ":: kirb, ehilwyma, opa334, 0x7ff, alfiecg25, sneko, sbingner\n"
        ":: nikias, tihmstar\n"
        ":: Checkra1n (Siguza, axi0mx, littlelailo et al.)\n"
        ":: Procursus (Hayden Seay, Cameron Katri, Keto et al.)\n"
        ":: ==========================>\n\n"
    );
}

// MARK: print_usage

void print_usage(char* argv) {
    printf(
        "Usage: %s [OPTIONS]\n\n"
        "  -h, --help       Print usage information\n"
        "  -v, --version    Print version\n\n"
        "Options:\n\n"
        "  --cli                                 Use command line interface\n"
        #ifdef WITH_GUI
        "  --gui                                 Use graphical user interface\n"
        #endif
        #ifdef WITH_TUI
        "  --tui                                 Use terminal user interface\n"
        #endif
        "  --dark-blockchain                     Enable dark blockchain\n"
        "  --force-revert                        Force environment reversion\n"
        "  --force-enable-ssv                    Force SSV detection to result in YES\n"
        // TODO: add --demote
        "  -l, --rootless                        Enable rootless mode (standard)\n"
        "  -f, --rootful                         Boots fakefs\n"
        "  -c, --setup-fakefs                    Setup fake filesystem\n"
        "  -B, --setup-partial-fakefs            Setup partial fake filesystem\n"
        "  -s, --safe-mode                       Enable safe mode\n"
        "  -T, --telnetd                         Enable TELNET daemon on port 46 (insecure)\n"
        "  -V, --verbose-boot                    Enable verbose booting\n"
        "  -p, --early-exit                      Exit after uploading Pongo\n"
        "  -e, --extra-bootargs <BOOTARGS>       Set extra bootargs\n"
        "  -k, --override-pongo <FILE PATH>      Override Pongo image\n"
        "  -K, --override-kpf <FILE PATH>        Override kernel patchfinder\n"
        "  -o, --override-overlay <FILE PATH>    Override overlay\n"
        "  -r, --override-ramdisk <FILE PATH>    Override ramdisk\n"
        "  -d, --debug-logging                   Enable debug logging\n"
        "  -n, --no-colors                       Disable colors on the command line\n"
        #if defined(WITH_GUI) || defined(WITH_TUI)
        "  -q, --quick                           Enable Quick Mode\n"
        #endif
        "\nEnvironment Variables:\n\n"
        #if WITH_CIDERRAIN
        "  RA1N_ABORT_TIMEOUT=1000000            Custom timeout value for the exploit abort timer.\n"
        #endif
        "  RA1N_CLI=1                            Use command line interface\n"
        #ifdef WITH_GUI
        "  RA1N_GUI=1                            Use graphical user interface\n"
        #endif
        #ifdef WITH_TUI
        "  RA1N_TUI=1                            Use terminal user interface\n"
        #endif
        "  RA1N_DARK_BLOCKCHAIN=1                Enable dark blockchain\n"
        "  RA1N_FORCE_REVERT=1                   Force environment reversion\n"
        "  RA1N_FORCE_ENABLE_SSV=1               Force SSV detection to result in YES\n"
        // TODO: add --demote
        "  RA1N_ROOTLESS=1                       Enable rootless mode (standard)\n"
        "  RA1N_ROOTFUL=1                        Boots fakefs\n"
        "  RA1N_SETUP_FAKEFS=1                   Setup fake filesystem\n"
        "  RA1N_SETUP_PARTIAL_FAKEFS=1           Setup partial fake filesystem\n"
        "  RA1N_SAFE_MODE=1                      Enable safe mode\n"
        "  RA1N_TELNETD=1                        Enable TELNET daemon on port 46 (insecure)\n"
        "  RA1N_VERBOSE_BOOT=1                   Enable verbose booting\n"
        "  RA1N_EARLY_EXIT=1                     Exit after uploading Pongo\n"
        "  RA1N_EXTRA_BOOTARGS=<FILE PATH>       Set extra bootargs\n"
        "  RA1N_OVERRIDE_PONGO=<FILE PATH>       Override Pongo image\n"
        "  RA1N_OVERRIDE_KPF=<FILE PATH>         Override kernel patchfinder\n"
        "  RA1N_OVERRIDE_OVERLAY=<FILE PATH>     Override overlay\n"
        "  RA1N_OVERRIDE_RAMDISK=<FILE PATH>     Override ramdisk\n"
        "  RA1N_DEBUG_LOGGING=1                  Enable debug logging\n"
        "  RA1N_NO_COLORS=1                      Disable colors on the command line\n"
        #if defined(WITH_GUI) || defined(WITH_TUI)
        "  RA1N_QUICK=1                          Enable Quick Mode\n"
        #endif
        , argv
    );

    printf(
        "\n"
        "Many things included in this program are made and developed by the Checkra1n team.\n"
        "Please do NOT contact the Checkra1n team for Palera1n, they are not responsible for this program.\n"
        "The original Checkra1n payloads are from: https://checkra.in/1337\n"
    );
}

// MARK: parse_arguments

static int get_env_binary(const char *env_name) {
    const char *val = getenv(env_name);
    if (!val) return -1;
    if (strcmp(val, "1") == 0) return 1;
    if (strcmp(val, "0") == 0) return 0;
    return -1;
}

void parse_arguments(int argc, char* argv[]) {
    int options;
    int option_index = 0;

    bool set_cli = false, set_gui = false, set_tui = false;
    bool set_flower = false, set_force_revert = false, set_ssv = false;
    bool set_rootless = false, set_rootful = false;
    bool set_fakefs = false, set_partial_fakefs = false;
    bool set_safemode = false, set_telnetd = false, set_verbose = false;
    bool set_pongo_exit = false, set_no_colors = false, set_quick = false;
    bool set_extra_bootargs = false, set_pongo = false, set_kpf = false;
    bool set_overlay = false, set_ramdisk = false;

    static struct option long_options[] = {
        {"help", no_argument, NULL, 'h'},
        {"version", no_argument, NULL, 'v'},
        {"cli", no_argument, NULL, 1},
        #ifdef WITH_GUI
        {"gui", no_argument, NULL, 2},
        #endif
        #ifdef WITH_TUI
        {"tui", no_argument, NULL, 3},
        #endif
        {"dark-blockchain", no_argument, NULL, 4},
        {"force-revert", no_argument, NULL, 5},
        {"force-enable-ssv", no_argument, NULL, 6},
        // TODO: add --demote
        {"rootless", no_argument, NULL, 'l'},
        {"rootful", no_argument, NULL, 'f'},
        {"setup-fakefs", no_argument, NULL, 'c'},
        {"setup-partial-fakefs", no_argument, NULL, 'B'},
        {"safe-mode", no_argument, NULL, 's'},
        {"telnetd", no_argument, NULL, 'T'},
        {"verbose-boot", no_argument, NULL, 'V'},
        {"early-exit", no_argument, NULL, 'p'},
        {"extra-bootargs", required_argument, NULL, 'e'},
        {"override-pongo", required_argument, NULL, 'k'},
        {"override-kpf", required_argument, NULL, 'K'},
        {"override-overlay", required_argument, NULL, 'o'},
        {"override-ramdisk", required_argument, NULL, 'r'},
        {"debug-logging", no_argument, NULL, 'd'},
        {"no-colors", no_argument, NULL, 'n'},
        #if defined(WITH_GUI) || defined(WITH_TUI)
        {"quick", no_argument, NULL, 'q'},
        #endif
        {NULL, 0, NULL, 0}
    };

    #ifndef _WIN32
    if (isatty(STDIN_FILENO) && getenv("LLVM_PROFILE_FILE") == nullptr) {
        #ifdef WITH_TUI
        palerain_flags |= palerain_option_tui;
        #else
        palerain_flags |= palerain_option_cli;
        #endif
    } else {
        #ifdef WITH_GUI
        palerain_flags |= palerain_option_gui;
        // xcode doesn't know how to display colors
        palerain_flags |= palerain_option_no_colors;
        #else
        palerain_flags |= palerain_option_cli;
        #endif
    }
    #else
    # ifdef WITH_GUI
    palerain_flags |= palerain_option_gui;
    # else
    palerain_flags |= palerain_option_cli;
    # endif
    #endif

    // MARK: Standard

    while ((options = getopt_long(argc, argv, "hvlfcBsTVpe:k:K:o:r:dnq", long_options, &option_index)) != -1) {
        switch (options) {
            case 'h': // --help
                print_usage(argv[0]);
                exit(1);
            case 'v': // --version
                #if WITH_CIDERRAIN
                printf("Palera1n " PALERAIN_VERSION " [USB: %s (libcidera1n %s)]\n",
                    ra1n_show_usb_backend(),
                    ra1n_show_build_version()
                );
                #else
                printf("Palera1n " PALERAIN_VERSION " [USB: %s (libopenra1n)]\n",
                    #ifdef __APPLE__
                    "IOKit"
                    #else
                    "libusb"
                    #endif
                );
                #endif
                printf("%s (%s)\n", GIT_HASH, GIT_BRANCH);
                printf("Git: %s\n", GIT_URL);
                printf("Compiled on %s at %s\n", __DATE__, __TIME__);
                exit(0);
            case 1: // --cli
                palerain_flags &= ~palerain_option_tui;
                palerain_flags &= ~palerain_option_gui;
                palerain_flags |= palerain_option_cli;
                break;
            #ifdef WITH_GUI
            case 2: // --gui
                palerain_flags &= ~palerain_option_cli;
                palerain_flags &= ~palerain_option_tui;
                palerain_flags |= palerain_option_gui;
                break;
            #endif
            #ifdef WITH_TUI
            case 3: // --tui
                palerain_flags &= ~palerain_option_cli;
                palerain_flags &= ~palerain_option_gui;
                palerain_flags |= palerain_option_tui;
                break;
            #endif
            case 4: // --dark-blockchain
                palerain_flags |= palerain_option_flower_chain;
                break;
            case 5: // --force-revert
                palerain_flags |= palerain_option_force_revert;
                break;
            case 6: // --force-enable-ssv
                palerain_flags |= palerain_option_ssv;
                break;
            case 'l': // --rootless
                palerain_flags &= ~palerain_option_rootful;
                palerain_flags |= palerain_option_rootless;
                break;
            case 'f': // --rootful
                palerain_flags &= ~palerain_option_rootless;
                palerain_flags |= palerain_option_rootful;
                break;
            case 'c': // --setup-fakefs
                palerain_flags |= palerain_option_setup_rootful;
                break;
            case 'B': // --setup-partial-fakefs
                palerain_flags |= palerain_option_setup_partial_root;
                break;
            case 's': // --safe-mode
                palerain_flags |= palerain_option_safemode;
                break;
            case 'T': // --telnetd
                palerain_flags |= palerain_option_telnetd;
                break;
            case 'V': // --verbose-boot
                palerain_flags |= palerain_option_verbose_boot;
                break;
            case 'p': // --early-exit
                palerain_flags |= palerain_option_pongo_exit;
                break;
            case 'e': // --extra-bootargs
                if (strlen(optarg) > (sizeof(boot_args) - 0x20)) {
                    LOG_ERROR("Boot arguments too long!");
                    exit(1);
                }

                // TODO: do some more checks on bootargs, like if it already
                // contains "palerain_flags" or things like "wdt" on rootful
                // using "-v" is also not supported

                // TODO: maybe automatically "wdt=-1" when rootful, and display
                // it through the various interfaces
                snprintf(boot_args, sizeof(boot_args), "%s", optarg);
                break;
            case 'k': // --override-pongo
                if (!override_payload_from_file(optarg, &g_payload_pongo)) {
                    LOG_ERROR("Failed to load pongo payload, is the path correct?");
                    exit(1);
                }
                if (g_payload_pongo.data_len > PONGO_MAX_SZ) {
                    LOG_ERROR("Pongo payload is too large! %zu bytes (max %zu)", g_payload_pongo.data_len, PONGO_MAX_SZ);
                    exit(1);
                }
                // TODO: check if macho, otherwise exit
                LOG("Overriding pongo payload with %s", optarg);
                break;
            case 'K': // --override-kpf
                if (!override_payload_from_file(optarg, &g_payload_kpf)) {
                    LOG_ERROR("Failed to load kpf payload, is the path correct?");
                    exit(1);
                }
                if (g_payload_kpf.data_len < 4
                    || (memcmp(g_payload_kpf.data, MACHO_MAGIC_64, 4) != 0
                    && memcmp(g_payload_kpf.data, MACHO_MAGIC_32, 4) != 0))
                {
                    LOG_ERROR("Invalid kpf payload, is it macho?");
                    exit(1);
                }
                LOG("Overriding kpf payload with %s", optarg);
                break;
            case 'o': // --override-overlay
                if (!override_payload_from_file(optarg, &g_payload_overlay)) {
                    LOG_ERROR("Failed to load overlay payload, is the path correct?");
                    exit(1);
                }
                LOG("Overriding overlay payload with %s", optarg);
                break;
            case 'r': // --override-ramdisk
                if (!override_payload_from_file(optarg, &g_payload_ramdisk)) {
                    LOG_ERROR("Failed to load ramdisk payload, is the path correct?");
                    exit(1);
                }
                LOG("Overriding ramdisk payload with %s", optarg);
                break;
            case 'd': // --debug-logging
                if (gDebugLevel < 5) gDebugLevel++;
                break;
            case 'n': // --no-colors
                palerain_flags |= palerain_option_no_colors;
                break;
            #if defined(WITH_GUI) || defined(WITH_TUI)
            case 'q': // --quick
                palerain_flags |= palerain_option_quick;
                break;
            #endif
            case '?':
                LOG_ERROR("Unknown option\n");
                print_usage(argv[0]);
                exit(1);
            default: break;
        }
    }

    // MARK: Environment vars

    if (!set_cli && get_env_binary("RA1N_CLI") == 1) {
        palerain_flags &= ~palerain_option_tui;
        palerain_flags &= ~palerain_option_gui;
        palerain_flags |= palerain_option_cli;
    }
    #ifdef WITH_GUI
    if (!set_gui && get_env_binary("RA1N_GUI") == 1) {
        palerain_flags &= ~palerain_option_cli;
        palerain_flags &= ~palerain_option_tui;
        palerain_flags |= palerain_option_gui;
    }
    #endif
    #ifdef WITH_TUI
    if (!set_tui && get_env_binary("RA1N_TUI") == 1) {
        palerain_flags &= ~palerain_option_cli;
        palerain_flags &= ~palerain_option_gui;
        palerain_flags |= palerain_option_tui;
    }
    #endif
    if (!set_flower && get_env_binary("RA1N_DARK_BLOCKCHAIN") == 1) {
        palerain_flags |= palerain_option_flower_chain;
    }
    if (!set_force_revert && get_env_binary("RA1N_FORCE_REVERT") == 1) {
        palerain_flags |= palerain_option_force_revert;
    }
    if (!set_ssv && get_env_binary("RA1N_FORCE_ENABLE_SSV") == 1) {
        palerain_flags |= palerain_option_ssv;
    }
    if (!set_rootless && get_env_binary("RA1N_ROOTLESS") == 1) {
        palerain_flags &= ~palerain_option_rootful;
        palerain_flags |= palerain_option_rootless;
    }
    if (!set_rootful && get_env_binary("RA1N_ROOTFUL") == 1) {
        palerain_flags &= ~palerain_option_rootless;
        palerain_flags |= palerain_option_rootful;
    }
    if (!set_fakefs && get_env_binary("RA1N_SETUP_FAKEFS") == 1) {
        palerain_flags |= palerain_option_setup_rootful;
    }
    if (!set_partial_fakefs && get_env_binary("RA1N_SETUP_PARTIAL_FAKEFS") == 1) {
        palerain_flags |= palerain_option_setup_partial_root;
    }
    if (!set_safemode && get_env_binary("RA1N_SAFE_MODE") == 1) {
        palerain_flags |= palerain_option_safemode;
    }
    if (!set_telnetd && get_env_binary("RA1N_TELNETD") == 1) {
        palerain_flags |= palerain_option_telnetd;
    }
    if (!set_verbose && get_env_binary("RA1N_VERBOSE_BOOT") == 1) {
        palerain_flags |= palerain_option_verbose_boot;
    }
    if (!set_pongo_exit && get_env_binary("RA1N_EARLY_EXIT") == 1) {
        palerain_flags |= palerain_option_pongo_exit;
    }
    if (!set_no_colors && get_env_binary("RA1N_NO_COLORS") == 1) {
        palerain_flags |= palerain_option_no_colors;
    }
    #if defined(WITH_GUI) || defined(WITH_TUI)
    if (!set_quick && get_env_binary("RA1N_QUICK") == 1) {
        palerain_flags |= palerain_option_quick;
    }
    #endif

    const char *env_debug = getenv("RA1N_DEBUG_LOGGING");
    if (env_debug) {
        int lvl = atoi(env_debug);
        if (lvl > 0) {
            gDebugLevel = (lvl > 5) ? 5 : lvl;
        }
    }

    if (!set_extra_bootargs) {
        const char *env_bootargs = getenv("RA1N_EXTRA_BOOTARGS");
        if (env_bootargs) {
            if (strlen(env_bootargs) > (sizeof(boot_args) - 0x20)) {
                LOG_ERROR("Boot arguments too long!");
                exit(1);
            }
            snprintf(boot_args, sizeof(boot_args), "%s", env_bootargs);
        }
    }

    if (!set_pongo) {
        const char *env_pongo = getenv("RA1N_OVERRIDE_PONGO");
        if (env_pongo) {
            if (!override_payload_from_file(env_pongo, &g_payload_pongo)) {
                LOG_ERROR("Failed to load pongo payload from env, is the path correct?");
                exit(1);
            }
            if (g_payload_pongo.data_len > PONGO_MAX_SZ) {
                LOG_ERROR("Pongo payload is too large! %zu bytes (max %zu)", g_payload_pongo.data_len, PONGO_MAX_SZ);
                exit(1);
            }
            LOG("Overriding pongo payload with %s (from env)", env_pongo);
        }
    }

    if (!set_kpf) {
        const char *env_kpf = getenv("RA1N_OVERRIDE_KPF");
        if (env_kpf) {
            if (!override_payload_from_file(env_kpf, &g_payload_kpf)) {
                LOG_ERROR("Failed to load kpf payload from env, is the path correct?");
                exit(1);
            }
            if (g_payload_kpf.data_len < 4
                || (memcmp(g_payload_kpf.data, MACHO_MAGIC_64, 4) != 0
                && memcmp(g_payload_kpf.data, MACHO_MAGIC_32, 4) != 0))
            {
                LOG_ERROR("Invalid kpf payload from env, is it macho?");
                exit(1);
            }
            LOG("Overriding kpf payload with %s (from env)", env_kpf);
        }
    }

    if (!set_overlay) {
        const char *env_overlay = getenv("RA1N_OVERRIDE_OVERLAY");
        if (env_overlay) {
            if (!override_payload_from_file(env_overlay, &g_payload_overlay)) {
                LOG_ERROR("Failed to load overlay payload from env, is the path correct?");
                exit(1);
            }
            LOG("Overriding overlay payload with %s (from env)", env_overlay);
        }
    }

    if (!set_ramdisk) {
        const char *env_ramdisk = getenv("RA1N_OVERRIDE_RAMDISK");
        if (env_ramdisk) {
            if (!override_payload_from_file(env_ramdisk, &g_payload_ramdisk)) {
                LOG_ERROR("Failed to load ramdisk payload from env, is the path correct?");
                exit(1);
            }
            LOG("Overriding ramdisk payload with %s (from env)", env_ramdisk);
        }
    }

    // MARK: Flag checks

    if (!(palerain_flags & palerain_option_gui) &&
        !(palerain_flags & palerain_option_tui))
    {
        if ((palerain_flags & palerain_option_pongo_exit) &&
            ((palerain_flags & palerain_option_rootful) ||
            (palerain_flags & palerain_option_rootless) ||
            (palerain_flags & palerain_option_setup_rootful) ||
            (palerain_flags & palerain_option_setup_partial_root)))
        {
            LOG_ERROR("[-p, --early-exit] cannot be used with [-f, --rootful], [-l, --rootless], [-c, --setup-fakefs], or [-B, --setup-partial-fakefs].\n");
            print_usage(argv[0]);
            exit(1);
        }

        if (!(palerain_flags & palerain_option_pongo_exit) &&
            !(palerain_flags & palerain_option_rootful) &&
            !(palerain_flags & palerain_option_rootless))
        {
            LOG_ERROR("You must specify either [-l, --rootless] or [-f, --rootful].\n");
            print_usage(argv[0]);
            exit(1);
        }

        if ((palerain_flags & palerain_option_rootful) &&
            (palerain_flags & palerain_option_rootless))
        {
            LOG_ERROR("You cannot specify both [-l, --rootless] and [-f, --rootful].\n");
            print_usage(argv[0]);
            exit(1);
        }

        if ((palerain_flags & palerain_option_setup_rootful) &&
            (palerain_flags & palerain_option_setup_partial_root))
        {
            LOG_ERROR("You cannot specify both [-c, --setup-fakefs] and [-B, --setup-partial-fakefs].\n");
            print_usage(argv[0]);
            exit(1);
        }

        if (((palerain_flags & palerain_option_setup_rootful) ||
            (palerain_flags & palerain_option_setup_partial_root)) &&
            !(palerain_flags & palerain_option_rootful))
        {
            LOG_ERROR("[-c, --setup-fakefs] or [-B, --setup-partial-fakefs] require [-f, --rootful].\n");
            print_usage(argv[0]);
            exit(1);
        }
    }

    // controls logging settings in our checkm8 libraries
    if (palerain_flags & palerain_option_no_colors) gColoredLogs = false;
    if (palerain_flags & palerain_option_tui) gSilentLogs = true;
}

// MARK: main

int main(int argc, char* argv[], char* envp[]) {
    print_credits();
    parse_arguments(argc, argv);

    LOG("Welcome to palera1n!");
    LOG("Reminder, this jailbreak IS designed for iOS 15+");
    LOG_DEBUG("palera1n_flags: 0x%" PRIx64, palerain_flags);

    // communicating with libusb on linux needs root
    #ifdef __linux__
    if (geteuid() != 0) {
        LOG_ERROR("You are not running as root, this may cause issues when exploiting...");
    }
    #endif

    #ifdef WITH_GUI
    if (palerain_flags & palerain_option_gui) {
        wxEntryStart(argc, argv);
        wxTheApp->CallOnInit();
        wxTheApp->OnRun();
        wxTheApp->OnExit();
        wxEntryCleanup();
        return 0;
    }
    #endif

    #ifdef WITH_TUI
    if (palerain_flags & palerain_option_tui) {
        // if terminfo is not set, set one
        if (getenv("TERMINFO") == nullptr) {
            // (at least on my environment) the terminal would fail to run
            // with sudo unless this is set, so if its not set correctly
            // just use a standard path on lunix
            setenv("TERMINFO", "/usr/share/terminfo", 1);
        }
        ui_run();
        return 0;
    }
    #endif

    int stage;
    bool ret = exploit(&stage);
    LOG("Thank you for using palera1n!");
    return ret;
}

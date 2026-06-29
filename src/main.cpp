#include <unistd.h>
#include <cstdlib>
#include <iostream>
#include <getopt.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#ifdef WITH_GUI
# include <wx/wx.h>
#endif

#include "m8/run.h"

#include "utils.h"
#include "globals.h"
#include "paleinfo.h"
#ifdef _WIN32
# include "usb/driver.h"
#endif

void print_credits() {
    printf(
        "::\n"
        ":: Palera1n beta " PALERAIN_VERSION "\n"
        "::\n"
        ":: (c) 2026\n"
        "::\n"
        ":: ========  Made by  =======>\n"
        ":: Made by: asdfugil, kok3shidoll, claration, mineek\n"
        ":: nekohaxx, plooshi, staturnz\n"
        ":: ======== Thanks to =======>\n"
        ":: Thanks to: llsc12, itsnebulalol, lrdsnow, kirb, ehilwyma\n"
        ":: opa334, 0x7ff, sbingner, nikias (libimobiledevice)\n"
        ":: dedbeddedbed, tihmstar\n"
        ":: Checkra1n (Siguza, axi0mx, littlelailo et al.)\n"
        ":: Procursus (Hayden Seay, Cameron Katri, Keto et al.)\n"
        ":: ==========================>\n\n"
    );
}

void print_usage(char* argv) {
    printf(
        "Usage: %s [OPTIONS]\n\n"
        "  -h, --help       Print usage information\n"
        "  -v, --version    Print version\n\n"
        "Options:\n\n"
        "  --cli                                 [f] Use command line interface\n"
        #ifdef WITH_GUI
        "  --gui                                 [f] Use graphical user interface\n"
        #endif
        #ifdef WITH_TUI
        "  --tui                                 [f] Use terminal user interface\n"
        #endif
        "  --dark-blockchain                     [f] Enable dark blockchain\n"
        "  --force-revert                        [f] Force environment reversion\n"
        "  -l, --rootless                        [f] Enable rootless mode (standard)\n"
        "  -f, --rootful                         [f] Boots fakefs\n"
        "  -c, --setup-fakefs                    [f] Setup fake filesystem\n"
        "  -B, --setup-partial-fakefs            [f] Setup partial fake filesystem\n"
        "  -s, --safe-mode                       [f] Enable safe mode\n"
        "  -T, --telnetd                         [f] Enable TELNET daemon on port 46 (insecure)\n"
        "  -V, --verbose-boot                    [f] Enable verbose booting\n"
        "  -p, --early-exit                      [f] Exit after uploading Pongo\n"
        "  -e, --extra-bootargs <BOOTARGS>       Set extra bootargs\n"
        "  -k, --override-pongo <FILE PATH>      Override Pongo image\n"
        "  -K, --override-kpf <FILE PATH>        Override kernel patchfinder\n"
        "  -o, --override-overlay <FILE PATH>    Override overlay\n"
        "  -r, --override-ramdisk <FILE PATH>    Override ramdisk\n"
        "  -n, --no-colors                       [f] Disable colors on the command line\n"
        "  -q, --quick                           [f] Enable Quick Mode\n"
        #ifdef _WIN32
        "  --install-drivers <PID>               Install libusbK drivers for PID\n"
        "  --remove-drivers <PID>                Remove libusbK drivers for PID\n"
        #endif
        , argv
    );
}

void parse_arguments(int argc, char* argv[]) {
    int options;
    int option_index = 0;

    static struct option long_options[] = {
        {"help", no_argument, NULL, 'h'},
        {"version", no_argument, NULL, 'v'},
        {"cli", no_argument, NULL, 1},
        #ifdef WITH_GUI
        {"gui", no_argument, NULL, 2},
        #endif
        #ifdef WITH_GUI
        {"tui", no_argument, NULL, 3},
        #endif
        {"dark-blockchain", no_argument, NULL, 4},
        {"force-revert", no_argument, NULL, 5},
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
        {"no-colors", no_argument, NULL, 'n'},
        {"quick", no_argument, NULL, 'q'},
        #ifdef _WIN32
        {"install-drivers", required_argument, NULL, 6},
        {"remove-drivers", required_argument, NULL, 7},
        #endif
        {NULL, 0, NULL, 0}
    };

    #ifdef WITH_GUI
    palerain_flags |= palerain_option_gui;
    # ifndef _WIN32
    if (isatty(STDIN_FILENO) && getenv("LLVM_PROFILE_FILE") == nullptr) {
        #ifdef WITH_TUI
        palerain_flags &= ~palerain_option_gui;
        palerain_flags |= palerain_option_tui;
        #else
        palerain_flags &= ~palerain_option_gui;
        palerain_flags |= palerain_option_cli;
        #endif
    } else {
        palerain_flags &= ~palerain_option_cli;
        palerain_flags |= palerain_option_gui;
        // xcode doesn't know how to display colors
        palerain_flags |= palerain_option_no_colors;
    }
    # endif
    #else
    palerain_flags |= palerain_option_cli;
    #endif

    while ((options = getopt_long(argc, argv, "hvlfcB:sTVpe:k:K:o:r:nq", long_options, &option_index)) != -1) {
        switch (options) {
            case 'h': // --help
                print_usage(argv[0]);
                exit(1);
            case 'v': // --version
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
                    LOG_ERROR("Boot arguments too long");
                    exit(1);
                }
                snprintf(boot_args, sizeof(boot_args), "%s", optarg);
                break;
            case 'k': // --override-pongo
                if (!override_payload_from_file(optarg, &g_payload_pongo)) {
                    LOG_ERROR("Failed to load pongo payload\n");
                    exit(1);
                }
                LOG("Overriding pongo payload with %s", optarg);
                break;
            case 'K': // --override-kpf
                if (!override_payload_from_file(optarg, &g_payload_kpf)) {
                    LOG_ERROR("Failed to load kpf payload\n");
                    exit(1);
                }
                LOG("Overriding kpf payload with %s", optarg);
                break;
            case 'o': // --override-overlay
                if (!override_payload_from_file(optarg, &g_payload_overlay)) {
                    LOG_ERROR("Failed to load overlay payload\n");
                    exit(1);
                }
                LOG("Overriding overlay payload with %s", optarg);
                break;
            case 'r': // --override-ramdisk
                if (!override_payload_from_file(optarg, &g_payload_ramdisk)) {
                    LOG_ERROR("Failed to load ramdisk payload\n");
                    exit(1);
                }
                LOG("Overriding ramdisk payload with %s", optarg);
                break;
            case 'n': // --no-colors
                palerain_flags |= palerain_option_no_colors;
                break;
            case 'q': // --quick
                palerain_flags |= palerain_option_quick;
                break;
            #ifdef _WIN32
            case 6: { // --install-drivers <PID>
                unsigned short parsed_pid = (unsigned short)strtol(optarg, NULL, 16);
                if (parsed_pid == 0) {
                    LOG_ERROR("Invalid PID value provided\n");
                    exit(1);
                }
                install_libusbk_target(0x05AC, parsed_pid);
                exit(0);
            }
            case 7: { // --remove-drivers <PID>
                unsigned short parsed_pid = (unsigned short)strtol(optarg, NULL, 16);
                if (parsed_pid == 0) {
                    LOG_ERROR("Invalid PID value provided\n");
                    exit(1);
                }
                uninstall_libusbk_target(0x05AC, parsed_pid);
                exit(0);
            }
            #endif
            case '?':
                LOG_ERROR("Unknown option\n");
                print_usage(argv[0]);
                exit(1);
            default: break;
        }
    }

    if (!(palerain_flags & palerain_option_gui)) {
        if (!(palerain_flags & palerain_option_rootful) && !(palerain_flags & palerain_option_rootless)) {
            LOG_ERROR("You must specify either -l, --rootless or -f, --rootful.\n");
            print_usage(argv[0]);
            exit(1);
        }
    }
}

int main(int argc, char* argv[], char* envp[]) {
    print_credits();
    parse_arguments(argc, argv);
    LOG("palera1n_flags: %llu", palerain_flags);
    shared_t state{};

    #ifdef WITH_GUI
    if (palerain_flags & palerain_option_gui) {
        wxEntryStart(argc, argv);
        wxTheApp->CallOnInit();
        wxTheApp->OnRun();
        wxTheApp->OnExit();
        wxEntryCleanup();
        return 0;
    } else {
        return exploit(&state);
    }
    #else
    return exploit(&state);
    #endif

    return 0;
}

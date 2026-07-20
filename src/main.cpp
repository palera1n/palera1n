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
        ":: Thanks to: itsnebulalol, llsc12, lrdsnow, dedbeddedbed\n"
        ":: kirb, ehilwyma, opa334, 0x7ff, alfiecg25, sneko, sbingner\n"
        ":: nikias, tihmstar\n"
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
        "  --force-enable-ssv                    [f] Force SSV detection to result in YES\n"
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
        , argv
    );

    printf(
        "\n"
        "Many things included in this program are made and developed by the Checkra1n team.\n"
        "Please do NOT contact the Checkra1n team for Palera1n, they are not responsible for this program.\n"
        "The original Checkra1n payloads are from: https://checkra.in/1337\n"
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
        #ifdef WITH_TUI
        {"tui", no_argument, NULL, 3},
        #endif
        {"dark-blockchain", no_argument, NULL, 4},
        {"force-revert", no_argument, NULL, 5},
        {"force-enable-ssv", no_argument, NULL, 6},
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

    while ((options = getopt_long(argc, argv, "hvlfcBsTVpe:k:K:o:r:nq", long_options, &option_index)) != -1) {
        switch (options) {
            case 'h': // --help
                print_usage(argv[0]);
                exit(1);
            case 'v': // --version
                #if WITH_CIDERRAIN
                printf("Palera1n beta " PALERAIN_VERSION " [USB: %s (libcidera1n %s)]\n",
                    ra1n_show_usb_backend(),
                    ra1n_show_build_version()
                );
                #else
                printf("Palera1n beta " PALERAIN_VERSION " [USB: %s (libopenra1n)]\n",
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
                if (g_payload_pongo.data_len > PONGO_MAX_SZ) {
                    LOG_ERROR("Pongo payload too large: %zu bytes (max %zu)", g_payload_pongo.data_len, PONGO_MAX_SZ);
                    exit(1);
                }
                LOG("Overriding pongo payload with %s", optarg);
                break;
            case 'K': // --override-kpf
                if (!override_payload_from_file(optarg, &g_payload_kpf)) {
                    LOG_ERROR("Failed to load kpf payload\n");
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
            case '?':
                LOG_ERROR("Unknown option\n");
                print_usage(argv[0]);
                exit(1);
            default: break;
        }
    }

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
    if (palerain_flags & palerain_option_no_colors) gSilentLogs = false;
    if (palerain_flags & palerain_option_tui) gSilentLogs = true;
}

int main(int argc, char* argv[], char* envp[]) {
    print_credits();
    parse_arguments(argc, argv);
    LOG("palera1n_flags: %llu", palerain_flags);

    // communicating with libusb on linux needs root
    #ifdef __linux__
    if (geteuid() != 0) {
        LOG("You are not running as root, this may cause issues when exploiting");
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

    shared_t state{};
    return exploit(&state);
}

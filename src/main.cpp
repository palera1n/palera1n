#include <unistd.h> // isatty, STDIN_FILENO
#include <cstdlib>  // getenv
#include <iostream>
#include <getopt.h>
#include <cstdio>
#include <cstdlib>

#ifdef WITH_GUI
# include <wx/wx.h>
#endif

#include "m8/run.h"

#include "utils.h"
#include "paleinfo.h"

void print_credits() {
    printf(
        "::\n"
        ":: Palera1n beta " PALERAIN_VERSION "\n"
        "::\n"
        ":: (c) 2026\n"
        "::\n"
        ":: ========  Made by  ======== ::\n"
        ":: Made by: asdfugil, kok3shidoll, claration, mineek\n"
        ":: nekohaxx, plooshi, staturnz\n"
        ":: ======== Thanks to ======== ::\n"
        ":: Thanks to: llsc12, itsnebulalol, lrdsnow, kirb, ehilwyma\n"
        ":: opa334, 0x7ff, sbingner, nikias (libimobiledevice)\n"
        ":: dedbeddedbed, tihmstar\n"
        ":: Checkra1n (Siguza, axi0mx, littlelailo et al.)\n"
        ":: Procursus (Hayden Seay, Cameron Katri, Keto et al.)\n"
        ":: =========================== ::\n\n"
    );
}

void print_usage(char* argv) {
    printf(
        "Usage: %s [OPTIONS]\n\n"
        "  -h, --help       Print usage information\n"
        "  -v, --version    Print version\n\n"
        "Options:\n"
        "  --cli                         Use command line interface\n"
        #ifdef WITH_GUI
        "  --gui                         Use graphical user interface\n"
        #endif
        "  --force-revert                Force environment reversion\n"
        "  -l, --rootless                Enable rootless mode (standard)\n"
        "  -f, --rootful                 Enable rootful mode\n"
        "  -c, --setup-fakefs            Setup fake filesystem\n"
        "  -B, --setup-partial-fakefs    Setup partial fake filesystem\n"
        "  -s, --safe-mode               Enable safe mode\n"
        "  -T, --telnetd                 Enable TELNET daemon on port 46 (insecure)\n"
        "  -V, --verbose-boot            Enable verbose booting\n"
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
        {"gui", no_argument, NULL, 3},
        #endif
        {"force-revert", no_argument, NULL, 0},
        {"rootless", no_argument, NULL, 'l'},
        {"rootful", no_argument, NULL, 'f'},
        {"setup-fakefs", no_argument, NULL, 'c'},
        {"setup-partial-fakefs", no_argument, NULL, 'B'},
        {"safe-mode", no_argument, NULL, 's'},
        {"telnetd", no_argument, NULL, 'T'},
        {"verbose-boot", no_argument, NULL, 'V'},
        {NULL, 0, NULL, 0}
    };

    #ifdef WITH_GUI
    palerain_flags |= palerain_option_gui;
    # ifndef _WIN32
    if (isatty(STDIN_FILENO) && getenv("LLVM_PROFILE_FILE") == nullptr) {
        palerain_flags &= ~palerain_option_gui;
        palerain_flags |= palerain_option_cli;
    } else {
        palerain_flags &= ~palerain_option_cli;
        palerain_flags |= palerain_option_gui;
    }
    # endif
    #else
    palerain_flags |= palerain_option_cli;
    #endif

    while ((options = getopt_long(argc, argv, "hvlf:", long_options, &option_index)) != -1) {
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
            case 3: // --gui
                palerain_flags &= ~palerain_option_cli;
                palerain_flags &= ~palerain_option_tui;
                palerain_flags |= palerain_option_gui;
                break;
            #endif
            case 0: // --force-revert
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
            case '?':
                LOG_ERROR("Unknown option");
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

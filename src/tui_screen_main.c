#ifdef TUI
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <locale.h>
#include <unistd.h>
#include <assert.h>

#include <palerain.h>
#include <tui.h>

bool showing_ecid = false;

char palera1n_logo[17][16] = {
"       ++       ",
"      #@@#      ",
"     +@@@@+     ",
"    :@@@@@@:    ",
"    #@@@@@@#    ",
"   +@@@@@@@@+   ",
"  :#*@@@@@@*#:  ",
"  %###@@@@###%  ",
" .####%@@%####. ",
" @############@ ",
"=@@*########*@@=",
"+@@@@%####%@@@@+",
":@@@@%%**%%@@@@:",
" +@@%%*@@*%%@@+ ",        
"   ++%@@@@%++   ",
"      :++:      "
};

char random_quotes[][150] = {
    "panic(cpu 1 caller 0xfffffff0232bd958): SEP Panic",
    "Now with Apple TV support!",
    "checkra1n will open source in 2020",
    "Try running palera1n on a Intel 80486!",
    "\"iOS 15 with SSV is unhackable\", but here we are.",
    "initproc exited -- exit reason namespace 1 subcode0x7 description: none",
    "totally not checkra1n",
    "Because the command line wasn't easy enough",
    "Checkra1n wrapper?",
    ""
};

#define PALERA1N_URL "https://palera.in"

int tui_main_nav_selection = 1;
bool quick_mode_enabled = false;
bool tui_can_start = false;
int random_quote;

bool easter_egg = false;

void tui_screen_main_nav(void) {
    MOVETOT(80 - 52, 23);
    printf("[%c] %sQuick Mode " COLOR(FG_WHITE, BG_BLACK) " %s[ Options ]" COLOR(FG_WHITE, BG_BLACK) " %s[  Start  ]" COLOR(FG_WHITE, BG_BLACK) " %s[  Quit   ]" COLOR(FG_WHITE, BG_BLACK), quick_mode_enabled ? 'x' : ' ',
        tui_main_nav_selection == 0 ? COLOR(FG_BLACK, BG_BRIGHT_WHITE) : (
            (
                tui_mouse_x >= tui_x_offset + 80 - 49 && tui_mouse_x <= tui_x_offset + 80 - 49 + 10
            ) && (tui_mouse_y == tui_y_offset + 22) ? COLOR(FG_BLACK, BG_WHITE) : ""
        ),
        tui_main_nav_selection == 1 ? COLOR(FG_BLACK, BG_BRIGHT_WHITE) : (
            (
                tui_mouse_x >= tui_x_offset + 80 - 37 && tui_mouse_x <= tui_x_offset + 80 - 37 + 10
            ) && (tui_mouse_y == tui_y_offset + 22) ? COLOR(FG_BLACK, BG_WHITE) : ""
        ),
        tui_main_nav_selection == 2 ? COLOR(FG_BLACK, BG_BRIGHT_WHITE) : !tui_can_start ? (supports_bright_colors ? COLOR(FG_BRIGHT_BLACK, BG_BLACK) : COLOR(FG_BLUE, BG_BLACK)) : (
            (
                tui_mouse_x >= tui_x_offset + 80 - 25 && tui_mouse_x <= tui_x_offset + 80 - 25 + 10
            ) && (tui_mouse_y == tui_y_offset + 22) ? COLOR(FG_BLACK, BG_WHITE) : ""
        ),
        tui_main_nav_selection == 3 ? COLOR(FG_BLACK, BG_BRIGHT_WHITE) : (
            (
                tui_mouse_x >= tui_x_offset + 80 - 13 && tui_mouse_x <= tui_x_offset + 80 - 13 + 10
            ) && (tui_mouse_y == tui_y_offset + 22) ? COLOR(FG_BLACK, BG_WHITE) : ""
        )
    );
}

void tui_screen_main_redraw(void) {
    PRINTATT(3, 2, "Welcome to palera1n!");

    SETCOLOR(FG_WHITE, BG_BLACK);
    PRINTATT(3, 9, "Made by: Nick Chan, Ploosh, Khcrysalis, Mineek");
    PRINTATT(3, 10, "staturnz, kok3shidoll, ");
    if ((tui_mouse_y == tui_y_offset + 9 && tui_mouse_x >= tui_x_offset + 25 && tui_mouse_x <= tui_x_offset + 25 + 8) || easter_egg) {
        SETCOLORA(FG_BRIGHT_WHITE, BG_BLACK, BOLD);
    }
    printf("HAHALOSAH");
    RESETFONT;
    SETCOLOR(FG_WHITE, BG_BLACK);
    PRINTATT(3, 12, "Thanks to: llsc12, nebula,");
    PRINTATT(3, 13, "nikias (libimobiledevice), checkra1n");
    PRINTATT(3, 14, "(Siguza, axi0mx, littlelailo et al.), Procursus");
    PRINTATT(3, 15, "Team (Hayden Seay, Cameron Katri, Keto et.al),");
    PRINTATT(80 - (int)sizeof(PALERA1N_URL), 21, PALERA1N_URL);
    PRINTATT(80 - (int)sizeof("@palera1n"), 20, "@palera1n");
    PRINTATT(3, 20, "With <3 from HAHALOSAH");
    PRINTATT(3, 21, "Note: Backup your stuff. Use at your own risk.");
    DRAWLINET(2, 22, 78);
    DRAWLINET(2, 3, 50);

    if (supports_bright_colors) {
        SETCOLOR(FG_BRIGHT_BLACK, BG_BLACK);
    } else {
        SETCOLOR(FG_BLUE, BG_BLACK);
    }
    DRAWLINET(2, 11, 50);
    DRAWLINET(2, 16, 50);

    for (int y = 0; y < 16; y++) {
        PRINTATLENT(80 - 22, y + 4, palera1n_logo[y], 16);
    }

    SETCOLOR(FG_YELLOW, BG_BLACK);
    if (tui_connected_devices == NULL) {
        PRINTATT(3, 5, "Connect your iPhone, iPad, iPod Touch, or Apple");
        PRINTATT(3, 6, "TV to begin.");
        tui_can_start = false;
    } else if (tui_connected_devices->next != NULL) {
        PRINTATT(3, 5, "Please attach only one device.");
        tui_can_start = false;
    } else {
        if (strlen(tui_connected_devices->display_name) == 0) {
            PRINTATT(3, 5, "ERROR: Failed to open device in recovery/DFU mode");
            PRINTATT(3, 6, "Please reopen palera1n and try again.");
        } else if (tui_connected_devices->mode == TUI_DEVICE_MODE_NORMAL) {
            MOVETOT(3, 5);
            int ecid_y;
            if (!tui_connected_devices->arm64) {
                printf("Sorry, %s (iOS %s) is not supported.", tui_connected_devices->display_name, tui_connected_devices->version);
                ecid_y = 6;
                tui_can_start = false;
            } else if (tui_compare_versions(tui_connected_devices->version, "15.0") < 0) {
                printf("Sorry, %s is supported, but iOS", tui_connected_devices->display_name);
                MOVETOT(3, 6);
                printf("%s is not.", tui_connected_devices->version);
                PRINTATT(3, 7, "Supported versions are 15.0+");
                ecid_y = 8;
                tui_can_start = false;
             } else if (tui_connected_devices->requires_passcode_disabled) {
                printf("Sorry, this devices passcode must be disabled on %s.", tui_connected_devices->version);
                MOVETOT(3, 6);
                printf("On 16.0+, you must reset the phone to factory settings");
                MOVETOT(3, 7);
                printf("and not have a passcode enabled ever since last restore.");
                ecid_y = 8;
                tui_can_start = false;
            } else {
                printf("%s (%s) connected in Normal mode.", tui_connected_devices->display_name, tui_connected_devices->version);
                ecid_y = 6;
                tui_can_start = true;
            }
            if (showing_ecid) {
                MOVETOT(3, ecid_y);
                printf("ECID: 0x%" PRIx64 " ", tui_connected_devices->ecid);
            } else {
                PRINTATT(3, ecid_y, "Press (E) to show ECID.");
            }
        } else if (tui_connected_devices->mode == TUI_DEVICE_MODE_RECOVERY) {
            MOVETOT(3, 5);
            printf("%s connected in Recovery mode.", tui_connected_devices->display_name);
            if (showing_ecid) {
                MOVETOT(3, 6);
                printf("ECID: 0x%" PRIx64 " ", tui_connected_devices->ecid);
            } else {
                PRINTATT(3, 6, "Press (E) to show ECID.");
            }
            PRINTATT(3, 7, "NOTE: Ensure installed iOS is in range 15.0+");
            tui_can_start = true;
        } else if (tui_connected_devices->mode == TUI_DEVICE_MODE_DFU) {
            MOVETOT(3, 5);
            printf("%s connected in DFU mode.", tui_connected_devices->display_name);
            if (showing_ecid) {
                MOVETOT(3, 6);
                printf("ECID: 0x%" PRIx64 " ", tui_connected_devices->ecid);
            } else {
                PRINTATT(3, 6, "Press (E) to show ECID.");
            }
#ifdef DEV_BUILD
            tui_can_start = true;
#else
            PRINTATT(3, 7, "Please connect device in Normal/Recovery mode");
            PRINTATT(3, 8, "or run palera1n in CLI mode");
            tui_can_start = false;
#endif
        }
    }

    if (!tui_can_start && tui_main_nav_selection == 2) {
        tui_main_nav_selection = 1;
    }

    SETCOLOR(FG_WHITE, BG_BLACK);

    for (int y = 0; y < 3; y++) {
        PRINTATLEN(tui_x_offset + 3, tui_y_offset + 17 + y, random_quotes[random_quote] + y * 50, 50);
    }

    tui_screen_main_nav();
    fflush(stdout);
}

int tui_main_nav_mouse_select = -1;

tui_screen_t tui_screen_main(void) {
    random_quote = ((unsigned int)((rand()+rand())/2)) % 8;
    redraw_screen();

    while(1) {
        int event = tui_get_event();
        switch (event) {
            case TUI_EVENT_CONNECTED_DEVICES_CHANGED:
                redraw_screen();
                break;
            case TUI_EVENT_INPUT: {
                int input = tui_last_input;

                switch (input) {
                    case TUI_INPUT_NONE:
                        if (tui_last_key == 'E' || tui_last_key == 'e') {
                            showing_ecid = !showing_ecid;
                            tui_screen_main_redraw();
                        }
                        break;
                    case TUI_INPUT_LEFT:
                    case TUI_INPUT_TAB_BACK:
                        if (--tui_main_nav_selection == -1) tui_main_nav_selection = 3;
                        if (tui_main_nav_selection == 2 && !tui_can_start) tui_main_nav_selection = 1;
                        tui_screen_main_nav();
                        fflush(stdout);
                        break;
                    case TUI_INPUT_RIGHT:
                    case TUI_INPUT_TAB:
                        if (++tui_main_nav_selection == 4) tui_main_nav_selection = 0;
                        if (tui_main_nav_selection == 2 && !tui_can_start) tui_main_nav_selection = 3;
                        tui_screen_main_nav();
                        fflush(stdout);
                        break;
                    case TUI_INPUT_SELECT:
                        switch(tui_main_nav_selection) {
                        case 0:
                            quick_mode_enabled = !quick_mode_enabled;
                            tui_screen_main_nav();
                            fflush(stdout);
                            break;
                        case 1:
                            return OPTIONS_SCREEN;
                        case 2:
                            if (tui_can_start) {
                                if (tui_connected_devices->mode == TUI_DEVICE_MODE_NORMAL) {
                                    return ENTER_RECOVERY_SCREEN;
                                } else if (tui_connected_devices->mode == TUI_DEVICE_MODE_RECOVERY) {
                                    return ENTER_DFU_SCREEN;
                                } else if (tui_connected_devices->mode == TUI_DEVICE_MODE_DFU) {
                                    tui_jailbreak();
                                    return JAILBREAK_SCREEN;
                                }
                            }
                            break;
                        case 3:
                            return EXIT_SCREEN;
                        }
                        break;
                    case TUI_INPUT_MOUSE_MOVE:
                        tui_screen_main_redraw();
                        break;
                    case TUI_INPUT_MOUSE_DOWN:
                        if ((tui_mouse_y == tui_y_offset + 9 && tui_mouse_x >= tui_x_offset + 25 && tui_mouse_x <= tui_x_offset + 25 + 8)) {
                            easter_egg = true;
                            break;
                        }
                        if (tui_mouse_y == tui_y_offset + 22) {
                            if (tui_mouse_x >= tui_x_offset + 80 - 49 && tui_mouse_x <= tui_x_offset + 80 - 49 + 10) {
                                tui_main_nav_selection = 0;
                                tui_main_nav_mouse_select = 0;
                            } else if (tui_mouse_x >= tui_x_offset + 80 - 37 && tui_mouse_x <= tui_x_offset + 80 - 37 + 10) {
                                tui_main_nav_selection = 1;
                                tui_main_nav_mouse_select = 1;
                            } else if (tui_mouse_x >= tui_x_offset + 80 - 25 && tui_mouse_x <= tui_x_offset + 80 - 25 + 10) {
                                if (tui_can_start) {
                                    tui_main_nav_selection = 2;
                                    tui_main_nav_mouse_select = 2;
                                }
                            } else if (tui_mouse_x >= tui_x_offset + 80 - 13 && tui_mouse_x <= tui_x_offset + 80 - 13 + 10) {
                                tui_main_nav_selection = 3;
                                tui_main_nav_mouse_select = 3;
                            }
                            tui_screen_main_nav();
                            fflush(stdout);
                        }
                        break;
                    case TUI_INPUT_MOUSE_UP:
                        if (tui_mouse_y == tui_y_offset + 22) {
                            if (tui_mouse_x >= tui_x_offset + 80 - 49 && tui_mouse_x <= tui_x_offset + 80 - 49 + 10 && tui_main_nav_mouse_select == 0) {
                                quick_mode_enabled = !quick_mode_enabled;
                                redraw_screen();
                            } else if (tui_mouse_x >= tui_x_offset + 80 - 37 && tui_mouse_x <= tui_x_offset + 80 - 37 + 10 && tui_main_nav_mouse_select == 1) {
                                return OPTIONS_SCREEN;
                            } else if (tui_mouse_x >= tui_x_offset + 80 - 25 && tui_mouse_x <= tui_x_offset + 80 - 25 + 10 && tui_main_nav_mouse_select == 2) {
                                if (tui_can_start) {
                                    if (tui_connected_devices->mode == TUI_DEVICE_MODE_NORMAL) {
                                        return ENTER_RECOVERY_SCREEN;
                                    } else if (tui_connected_devices->mode == TUI_DEVICE_MODE_RECOVERY) {
                                        return ENTER_DFU_SCREEN;
                                    } else if (tui_connected_devices->mode == TUI_DEVICE_MODE_DFU) {
                                        tui_jailbreak();
                                        return JAILBREAK_SCREEN;
                                    }
                                }
                            } else if (tui_mouse_x >= tui_x_offset + 80 - 13 && tui_mouse_x <= tui_x_offset + 80 - 13 + 10 && tui_main_nav_mouse_select == 3) {
                                return EXIT_SCREEN;
                            }
                        }
                        tui_main_nav_mouse_select = -1;
                        break;
                }
                break;
            }
        }
        
    }

    return MAIN_SCREEN;
}

int tui_version_extract_component(const char *version, int index) {
    int component = 0;
    int componentIndex = 0;
    for (int i = 0; version[i] != '\0'; i++) {
        if (version[i] == '.') {
            componentIndex++;
            if (componentIndex > index) {
                break;
            }
            continue;
        }
        if (componentIndex == index) {
            component *= 10;
            component += version[i] - '0';
        }
    }
    return component;
}

int tui_compare_versions(const char *firstVersion, const char *secondVersion) {
    if (strcmp(firstVersion, secondVersion) == 0) {
        return 0;
    }

    int firstVersionMajor = tui_version_extract_component(firstVersion, 0);
    int firstVersionMinor = tui_version_extract_component(firstVersion, 1);
    int firstVersionPatch = tui_version_extract_component(firstVersion, 2);
    
    int secondVersionMajor = tui_version_extract_component(secondVersion, 0);
    int secondVersionMinor = tui_version_extract_component(secondVersion, 1);
    int secondVersionPatch = tui_version_extract_component(secondVersion, 2);
    
    if (firstVersionMajor != secondVersionMajor) {
        return firstVersionMajor - secondVersionMajor;
    }
    
    if (firstVersionMinor != secondVersionMinor) {
        return firstVersionMinor - secondVersionMinor;
    }
    
    return firstVersionPatch - secondVersionPatch;
}

#else
/* ISO C forbids an empty translation unit */
extern char** environ;
#endif

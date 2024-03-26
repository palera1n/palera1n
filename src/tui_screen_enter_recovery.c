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
#include <time.h>
#include <poll.h>
#include <palerain.h>
#include <tui.h>

int tui_enter_recovery_nav_selection = 1;
bool tui_device_is_entering_recovery = false;
bool tui_enter_recovery_mode_failed = false;

void tui_screen_enter_recovery_nav(void) {
    MOVETOT(80 - 22, 23);
    printf("%s[  Back  ]\033[37;40m %s[  Next  ]\033[37;40m",
        tui_device_is_entering_recovery ? "\033[90;40m" : tui_enter_recovery_nav_selection == 0 ? "\033[30;107m" : (
            (
                tui_mouse_x >= tui_x_offset + 80 - 23 && tui_mouse_x <= tui_x_offset + 80 - 23 + 10
            ) && (tui_mouse_y == tui_y_offset + 22) ? "\x1b[30;47m" : ""
        ),
        tui_device_is_entering_recovery ? "\033[90;40m" : tui_enter_recovery_nav_selection == 1 ? "\033[30;107m" : (
            (
                tui_mouse_x >= tui_x_offset + 80 - 12 && tui_mouse_x <= tui_x_offset + 80 - 12 + 10
            ) && (tui_mouse_y == tui_y_offset + 22) ? "\x1b[30;47m" : ""
        )
    );
}

char enter_recovery_loading[9][8] = {
    "........",
    "|.......",
    ".|......",
    "..|.....",
    "...|....",
    "....|...",
    ".....|..",
    "......|.",
    ".......|"
};

char enter_recovery_loading_troll[9][8] = {
    ".......",
    "t......",
    ".r.....",
    "..o....",
    "...l...",
    "....l..",
    ".....e.",
    "......d",
};

bool is_troll;
int tui_enter_recovery_loading_progress = 0;

void tui_screen_enter_recovery_loading(void) {
    SETCOLOR(FG_YELLOW, BG_BLACK);
    PRINTATT(3, 8, "Entering recovery mode ");
    printf("%.8s", is_troll ? enter_recovery_loading_troll[tui_enter_recovery_loading_progress] : enter_recovery_loading[tui_enter_recovery_loading_progress]);
    SETCOLOR(FG_WHITE, BG_BLACK);
    fflush(stdout);
}

void tui_screen_enter_recovery_redraw(void) {
    PRINTATT(3, 3, "The device needs to be put into DFU mode to apply the jailbreak. This is a");
    PRINTATT(3, 4, "manual process and we will guide you through it.");
    PRINTATT(3, 5, "In order to prevent filesystem corruption through hard reset, the device");
    PRINTATT(3, 6, "will be put into recovery mode first. Click Next when you are ready.");
    
    if (tui_enter_recovery_mode_failed) {
        SETCOLOR(FG_YELLOW, BG_BLACK);
        PRINTATT(3, 8, "Hmm... device did not enter recovery mode!? Click Next to try again...");
        SETCOLOR(FG_WHITE, BG_BLACK);
    }

    tui_screen_enter_recovery_nav();
    if (tui_device_is_entering_recovery) {
        tui_screen_enter_recovery_loading();
    }

    fflush(stdout);
}

int recovery_time = 0;

tui_screen_t tui_enter_recovery(void) {
    enter_recovery_cmd(tui_connected_devices->udid);
    is_troll = false;
    if (rand() % 1000 == 0) {
        is_troll = true;
    }
    int i = 0;
    recovery_time = 0;
    while(1) {
        redraw_screen();
        tui_enter_recovery_loading_progress++;
        tui_enter_recovery_loading_progress %= 9;
        // 100ms sleep
        struct timespec ts;
        ts.tv_sec = 0;
        ts.tv_nsec = 100000000;
        nanosleep(&ts, NULL);
        
        int event = tui_try_get_event();
        if (event == TUI_EVENT_CONNECTED_DEVICES_CHANGED) {
            if (tui_connected_devices && !tui_connected_devices->next && tui_connected_devices->mode == TUI_DEVICE_MODE_RECOVERY) {
                SETCOLOR(FG_YELLOW, BG_BLACK);
                PRINTATT(3, 8, "Device is now in recovery mode.");
                SETCOLOR(FG_WHITE, BG_BLACK);
                fflush(stdout);

                sleep(3);
                
                return ENTER_DFU_SCREEN;
            }
        }
        if (i > 60 * 10) {
            tui_enter_recovery_mode_failed = true;
            return ENTER_RECOVERY_SCREEN;
        }
    }
}

int tui_enter_recovery_nav_mouse_select = -1;

tui_screen_t tui_screen_enter_recovery(void) {
    tui_device_is_entering_recovery = false;
    tui_enter_recovery_loading_progress = 0;
    tui_enter_recovery_nav_selection = 1;
    redraw_screen();
    
    while(1) {
        int event = tui_get_event();
        switch (event) {
            case TUI_EVENT_CONNECTED_DEVICES_CHANGED:
                tui_enter_recovery_mode_failed = false;
                return MAIN_SCREEN;
            case TUI_EVENT_INPUT: {
                int input = tui_last_input;
                if (input == TUI_INPUT_NONE) continue;
                if (tui_device_is_entering_recovery) continue;

                switch (input) {
                    case TUI_INPUT_LEFT:
                    case TUI_INPUT_RIGHT:
                        tui_enter_recovery_nav_selection = !tui_enter_recovery_nav_selection;
                        redraw_screen();
                        break;
                    case TUI_INPUT_SELECT:
                        switch(tui_enter_recovery_nav_selection) {
                        case 0:
                            tui_enter_recovery_mode_failed = false;
                            return MAIN_SCREEN;
                        case 1:
                            tui_device_is_entering_recovery = true;
                            redraw_screen();
                            return tui_enter_recovery();
                        }
                    case TUI_INPUT_MOUSE_MOVE:
                        tui_screen_enter_recovery_nav();
                        fflush(stdout);
                        break;
                    case TUI_INPUT_MOUSE_DOWN:
                        if (tui_mouse_y == tui_y_offset + 22) {
                            if (tui_mouse_x >= tui_x_offset + 80 - 23 && tui_mouse_x <= tui_x_offset + 80 - 23 + 10) {
                                tui_enter_recovery_nav_selection = 0;
                                tui_enter_recovery_nav_mouse_select = 0;
                                tui_screen_enter_recovery_nav();
                                fflush(stdout);
                            } else if (tui_mouse_x >= tui_x_offset + 80 - 12 && tui_mouse_x <= tui_x_offset + 80 - 12 + 10) {
                                tui_enter_recovery_nav_selection = 1;
                                tui_enter_recovery_nav_mouse_select = 1;
                                tui_screen_enter_recovery_nav();
                                fflush(stdout);
                            }
                        }
                        break;
                    case TUI_INPUT_MOUSE_UP:
                        if (tui_mouse_y == tui_y_offset + 22) {
                            if (tui_mouse_x >= tui_x_offset + 80 - 23 && tui_mouse_x <= tui_x_offset + 80 - 23 + 10) {
                                return MAIN_SCREEN;
                            } else if (tui_mouse_x >= tui_x_offset + 80 - 12 && tui_mouse_x <= tui_x_offset + 80 - 12 + 10) {
                                tui_device_is_entering_recovery = true;
                                redraw_screen();
                                return tui_enter_recovery();
                            }
                        }
                        tui_enter_recovery_nav_mouse_select = -1;
                        break;
                }
                break;
            }
        }
    }
    tui_enter_recovery_mode_failed = false;
    return MAIN_SCREEN;
}
#else
/* ISO C forbids an empty translation unit */
extern char** environ;
#endif

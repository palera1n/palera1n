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

#include <palerain.h>
#include <tui.h>

int tui_enter_dfu_nav_selection = 1;
bool tui_device_is_entering_dfu = false;
int tui_enter_dfu_status = 0;

enum {
    DEVICE_TYPE_IPHONE_SE,
    DEVICE_TYPE_IPHONE_6S,
    DEVICE_TYPE_IPHONE_7_8,
    DEVICE_TYPE_IPHONE_X,
    DEVICE_TYPE_IPAD,
    DEVICE_TYPE_IPOD_TOUCH
} device_type;

int tui_product_type_to_device_type(char *product_type) {
    if (strcmp(product_type, "iPhone8,1") == 0 || strcmp(product_type, "iPhone8,2") == 0) {
        return DEVICE_TYPE_IPHONE_6S;
    }
    if (strcmp(product_type, "iPhone8,4") == 0) return DEVICE_TYPE_IPHONE_SE;
    if (strcmp(product_type, "iPhone9,1") == 0 || strcmp(product_type, "iPhone9,2") == 0
        || strcmp(product_type, "iPhone9,3") == 0 || strcmp(product_type, "iPhone9,4") == 0) {
        return DEVICE_TYPE_IPHONE_7_8;
    }
    if (strcmp(product_type, "iPhone10,1") == 0 || strcmp(product_type, "iPhone10,2") == 0
        || strcmp(product_type, "iPhone10,4") == 0 || strcmp(product_type, "iPhone10,5") == 0) {
        return DEVICE_TYPE_IPHONE_7_8;
    }
    if (strcmp(product_type, "iPhone10,3") == 0 || strcmp(product_type, "iPhone10,6") == 0) {
        return DEVICE_TYPE_IPHONE_X;
    }
    if (strncmp(product_type, "iPad", strlen("iPad")) == 0) {
        return DEVICE_TYPE_IPAD;
    }
    if (strcmp(product_type, "iPod9,1") == 0) {
        return DEVICE_TYPE_IPOD_TOUCH;
    }

    // Not found
    return DEVICE_TYPE_IPHONE_SE;
}

int tui_enter_dfu_device_type = DEVICE_TYPE_IPHONE_SE;

int tui_enter_dfu_loading_progress = 1;

void tui_screen_enter_dfu_nav(void) {
    MOVETOT(80 - 22, 23);
    printf("%s[ Cancel ]" COLOR(FG_WHITE, BG_BLACK) " %s[  %s ]" COLOR(FG_WHITE, BG_BLACK),
        tui_device_is_entering_dfu || tui_enter_dfu_status == 1 ? (supports_bright_colors ? COLOR(FG_BRIGHT_BLACK, BG_BLACK) : COLOR(FG_BLUE, BG_BLACK)) : tui_enter_dfu_nav_selection == 0 ? "\033[30;107m" : (
            (
                tui_mouse_x >= tui_x_offset + 80 - 23 && tui_mouse_x <= tui_x_offset + 80 - 23 + 10
            ) && (tui_mouse_y == tui_y_offset + 22) ? COLOR(FG_BLACK, BG_WHITE) : ""
        ),
        tui_device_is_entering_dfu || tui_enter_dfu_status == 1 ? (supports_bright_colors ? COLOR(FG_BRIGHT_BLACK, BG_BLACK) : COLOR(FG_BLUE, BG_BLACK)) : tui_enter_dfu_nav_selection == 1 ? "\033[30;107m" : (
            (
                tui_mouse_x >= tui_x_offset + 80 - 12 && tui_mouse_x <= tui_x_offset + 80 - 12 + 10
            ) && (tui_mouse_y == tui_y_offset + 22) ? COLOR(FG_BLACK, BG_WHITE) : ""
        ),
        tui_enter_dfu_status == 2 ? "Retry" : "Start"
    );
}

void tui_screen_enter_dfu_redraw(void) {
    SETCOLOR(FG_WHITE, BG_BLACK);
    switch(tui_enter_dfu_status) {
    case 0:
        PRINTATT(3, 3, "Time to put the device into DFU mode. Locate the buttons as marked below on");
        PRINTATT(3, 4, "your device and check the instructions on the right *before* clicking Start.");
        break;
    case 1:
        PRINTATT(3, 3, "Device entered DFU mode successfully.");
        break;
    case 2:
        PRINTATT(3, 3, "Whoops, the device didn't enter DFU mode. Click Retry to try again.");
        break;
    }
    
    tui_screen_enter_dfu_nav();

    switch(tui_enter_dfu_device_type) {
    case DEVICE_TYPE_IPHONE_SE:
    case DEVICE_TYPE_IPAD:
        if (tui_enter_dfu_loading_progress > 0 || tui_enter_dfu_status == 2) {
            if (supports_bright_colors) {
                SETCOLOR(FG_BRIGHT_BLACK, BG_BLACK);
            } else {
                SETCOLOR(FG_BLUE, BG_BLACK);
            }
        }
        PRINTATT(51, 6, "1. Click Start");
        if (tui_enter_dfu_loading_progress > 0) {
            SETCOLOR(FG_BRIGHT_WHITE, BG_BLACK);
        }

        if (!tui_device_is_entering_dfu || tui_enter_dfu_loading_progress > 4 || tui_enter_dfu_status == 2) {
            if (supports_bright_colors) {
                SETCOLOR(FG_BRIGHT_BLACK, BG_BLACK);
            } else {
                SETCOLOR(FG_BLUE, BG_BLACK);
            }
        }
        PRINTATT(51, 7, "2. Press and hold the Top and");
        MOVETOT(54, 8);
        printf("Home buttons together (%d)", tui_enter_dfu_loading_progress > 4 ? 0 : tui_enter_dfu_loading_progress < 1 ? 4 : 5 - tui_enter_dfu_loading_progress);
        if (!tui_device_is_entering_dfu || tui_enter_dfu_loading_progress > 4) {
            SETCOLOR(FG_BRIGHT_WHITE, BG_BLACK);
        }

        if (tui_enter_dfu_loading_progress < 5 || tui_enter_dfu_loading_progress > 14 || tui_enter_dfu_status == 2) {
            if (supports_bright_colors) {
                SETCOLOR(FG_BRIGHT_BLACK, BG_BLACK);
            } else {
                SETCOLOR(FG_BLUE, BG_BLACK);
            }
        }
        PRINTATT(51, 9, "3. Release the Top button BUT");
        PRINTATT(54, 10, "KEEP HOLDING the Home");
        MOVETOT(54, 11);
        printf("button (%d)", tui_enter_dfu_loading_progress > 14 ? 0 : tui_enter_dfu_loading_progress < 5 ? 10 : 15 - tui_enter_dfu_loading_progress);
        if (tui_enter_dfu_loading_progress < 5 || tui_enter_dfu_loading_progress > 14) {
            SETCOLOR(FG_BRIGHT_WHITE, BG_BLACK);
        }
        break;
    case DEVICE_TYPE_IPHONE_6S:
        if (tui_enter_dfu_loading_progress > 0 || tui_enter_dfu_status == 2) {
            if (supports_bright_colors) {
                SETCOLOR(FG_BRIGHT_BLACK, BG_BLACK);
            } else {
                SETCOLOR(FG_BLUE, BG_BLACK);
            }
        }
        PRINTATT(51, 6, "1. Click Start");
        if (tui_enter_dfu_loading_progress > 0) {
            SETCOLOR(FG_BRIGHT_WHITE, BG_BLACK);
        }

        if (!tui_device_is_entering_dfu || tui_enter_dfu_loading_progress > 4 || tui_enter_dfu_status == 2) {
            if (supports_bright_colors) {
                SETCOLOR(FG_BRIGHT_BLACK, BG_BLACK);
            } else {
                SETCOLOR(FG_BLUE, BG_BLACK);
            }
        }
        PRINTATT(51, 7, "2. Press and hold the Side");
        PRINTATT(54, 8, "and Home buttons together");
        MOVETOT(54, 9);
        printf("(%d)", tui_enter_dfu_loading_progress > 4 ? 0 : tui_enter_dfu_loading_progress < 1 ? 4 : 5 - tui_enter_dfu_loading_progress);
        if (!tui_device_is_entering_dfu || tui_enter_dfu_loading_progress > 4) {
            SETCOLOR(FG_BRIGHT_WHITE, BG_BLACK);
        }

        if (tui_enter_dfu_loading_progress < 5 || tui_enter_dfu_loading_progress > 14 || tui_enter_dfu_status == 2) {
            if (supports_bright_colors) {
                SETCOLOR(FG_BRIGHT_BLACK, BG_BLACK);
            } else {
                SETCOLOR(FG_BLUE, BG_BLACK);
            }
        }
        PRINTATT(51, 10, "3. Release the Side button");
        PRINTATT(54, 11, "BUT KEEP HOLDING the Home");
        MOVETOT(54, 12);
        printf("button (%d)", tui_enter_dfu_loading_progress > 14 ? 0 : tui_enter_dfu_loading_progress < 5 ? 10 : 15 - tui_enter_dfu_loading_progress);
        if (tui_enter_dfu_loading_progress < 5 || tui_enter_dfu_loading_progress > 14) {
            SETCOLOR(FG_BRIGHT_WHITE, BG_BLACK);
        }
        break;
    case DEVICE_TYPE_IPHONE_7_8:
    case DEVICE_TYPE_IPHONE_X:
        tui_draw_rectangle(17, 5, 33, 21);
        PRINTATT(17, 10, "|");
        MOVETOT(5, 11);
        printf("%sVolume down-", tui_enter_dfu_loading_progress > 14 ? (supports_bright_colors ? COLOR(FG_BRIGHT_BLACK, BG_BLACK) : COLOR(FG_BLUE, BG_BLACK)) : COLOR(FG_YELLOW, BG_BLACK));
        SETCOLOR(FG_BRIGHT_WHITE, BG_BLACK);
        putchar('|');
        DSGON;
        MOVETOT(35, 10);
        putchar(0x78);
        DSGOFF;
        printf("%s-Side button", tui_enter_dfu_loading_progress > 4 ? (supports_bright_colors ? COLOR(FG_BRIGHT_BLACK, BG_BLACK) : COLOR(FG_BLUE, BG_BLACK)) : COLOR(FG_YELLOW, BG_BLACK));
        SETCOLOR(FG_BRIGHT_WHITE, BG_BLACK);

        if (tui_enter_dfu_loading_progress > 0 || tui_enter_dfu_status == 2) {
            if (supports_bright_colors) {
                SETCOLOR(FG_BRIGHT_BLACK, BG_BLACK);
            } else {
                SETCOLOR(FG_BLUE, BG_BLACK);
            }
        }
        PRINTATT(51, 6, "1. Click Start");
        if (tui_enter_dfu_loading_progress > 0) {
            SETCOLOR(FG_BRIGHT_WHITE, BG_BLACK);
        }

        if (!tui_device_is_entering_dfu || tui_enter_dfu_loading_progress > 4 || tui_enter_dfu_status == 2) {
            if (supports_bright_colors) {
                SETCOLOR(FG_BRIGHT_BLACK, BG_BLACK);
            } else {
                SETCOLOR(FG_BLUE, BG_BLACK);
            }
        }
        PRINTATT(51, 7, "2. Press and hold the Side");
        PRINTATT(54, 8, "and Volume down buttons");
        MOVETOT(54, 9);
        printf("together (%d)", tui_enter_dfu_loading_progress > 4 ? 0 : tui_enter_dfu_loading_progress < 1 ? 4 : 5 - tui_enter_dfu_loading_progress);
        if (!tui_device_is_entering_dfu || tui_enter_dfu_loading_progress > 4) {
            SETCOLOR(FG_BRIGHT_WHITE, BG_BLACK);
        }

        if (tui_enter_dfu_loading_progress < 5 || tui_enter_dfu_loading_progress > 14 || tui_enter_dfu_status == 2) {
            if (supports_bright_colors) {
                SETCOLOR(FG_BRIGHT_BLACK, BG_BLACK);
            } else {
                SETCOLOR(FG_BLUE, BG_BLACK);
            }
        }
        PRINTATT(51, 10, "3. Release the Side button");
        PRINTATT(54, 11, "BUT KEEP HOLDING the");
        MOVETOT(54, 12);
        printf("Volume down button (%d)", tui_enter_dfu_loading_progress > 14 ? 0 : tui_enter_dfu_loading_progress < 5 ? 10 : 15 - tui_enter_dfu_loading_progress);
        if (tui_enter_dfu_loading_progress < 5 || tui_enter_dfu_loading_progress > 14) {
            SETCOLOR(FG_BRIGHT_WHITE, BG_BLACK);
        }
        break;
    }

    switch(tui_enter_dfu_device_type) {
    case DEVICE_TYPE_IPHONE_SE:
        tui_draw_rectangle(18, 6, 32, 21);
        tui_draw_rectangle(24, 19, 26, 20);
        for (int y = 9; y < 20; y++) {
            SETCOLOR(FG_BLACK, BG_BRIGHT_BLACK);
            PRINTATT(20, y, "             ");
            SETCOLOR(FG_BRIGHT_WHITE, BG_BLACK);
        }
        PRINTATT(23, 8, "o ===");
        PRINTATT(18, 11, "|");
        PRINTATT(18, 12, "|");
        MOVETOT(30, 6);
        printf("__ %s-Top button", tui_enter_dfu_loading_progress > 4 ? (supports_bright_colors ? COLOR(FG_BRIGHT_BLACK, BG_BLACK) : COLOR(FG_BLUE, BG_BLACK)) : COLOR(FG_YELLOW, BG_BLACK));
        SETCOLOR(FG_BRIGHT_WHITE, BG_BLACK);
        MOVETOT(31, 20);
        printf("%s-Home button", tui_enter_dfu_loading_progress > 14 ? (supports_bright_colors ? COLOR(FG_BRIGHT_BLACK, BG_BLACK) : COLOR(FG_BLUE, BG_BLACK)) : COLOR(FG_YELLOW, BG_BLACK));
        SETCOLOR(FG_BRIGHT_WHITE, BG_BLACK);

        break;
    case DEVICE_TYPE_IPHONE_6S:
        tui_draw_rectangle(17, 5, 33, 21);
        tui_draw_rectangle(24, 19, 26, 20);
        for (int y = 8; y < 20; y++) {
            SETCOLOR(FG_BLACK, BG_BRIGHT_BLACK);
            PRINTATT(19, y, "               ");
            SETCOLOR(FG_BRIGHT_WHITE, BG_BLACK);
        }
        PRINTATT(23, 7, "o ===");
        PRINTATT(17, 10, "|");
        PRINTATT(17, 11, "|");
        MOVETOT(35, 10);
        printf("|%s-Side button", tui_enter_dfu_loading_progress > 4 ? (supports_bright_colors ? COLOR(FG_BRIGHT_BLACK, BG_BLACK) : COLOR(FG_BLUE, BG_BLACK)) : COLOR(FG_YELLOW, BG_BLACK));
        SETCOLOR(FG_BRIGHT_WHITE, BG_BLACK);
        MOVETOT(30, 20);
        printf("%s-Home button", tui_enter_dfu_loading_progress > 14 ? (supports_bright_colors ? COLOR(FG_BRIGHT_BLACK, BG_BLACK) : COLOR(FG_BLUE, BG_BLACK)) : COLOR(FG_YELLOW, BG_BLACK));
        SETCOLOR(FG_BRIGHT_WHITE, BG_BLACK);
        break;
    case DEVICE_TYPE_IPHONE_7_8:
        tui_draw_rectangle(24, 19, 26, 20);
        for (int y = 8; y < 20; y++) {
            SETCOLOR(FG_BLACK, BG_BRIGHT_BLACK);
            PRINTATT(19, y, "               ");
            SETCOLOR(FG_BRIGHT_WHITE, BG_BLACK);
        }
        PRINTATT(23, 7, "o ===");

        break;
    case DEVICE_TYPE_IPHONE_X:
        SETCOLOR(FG_BLACK, BG_BRIGHT_BLACK);
        PRINTATT(19, 7, "    ");
        SETCOLOR(FG_BRIGHT_WHITE, BG_BLACK);
        printf("       ");
        SETCOLOR(FG_BLACK, BG_BRIGHT_BLACK);
        printf("    ");
        SETCOLOR(FG_BLACK, BG_BRIGHT_BLACK);
        for (int y = 8; y < 22; y++) {
            PRINTATT(19, y, "               ");
        }
        SETCOLOR(FG_BRIGHT_WHITE, BG_BLACK);

        DSGON;
        MOVETOT(35, 11);
        putchar(0x78);
        DSGOFF;
        
        break;
    case DEVICE_TYPE_IPAD:
        tui_draw_rectangle(14, 6, 36, 21);
        tui_draw_rectangle(24, 19, 26, 20);
        for (int y = 9; y < 20; y++) {
            SETCOLOR(FG_BLACK, BG_BRIGHT_BLACK);
            PRINTATT(16, y, "                     ");
            SETCOLOR(FG_BRIGHT_WHITE, BG_BLACK);
        }
        PRINTATT(23, 8, "o ===");
        PRINTATT(14, 11, "|");
        PRINTATT(14, 12, "|");
        MOVETOT(33, 6);
        printf("__  %s-Top button", tui_enter_dfu_loading_progress > 4 ? (supports_bright_colors ? COLOR(FG_BRIGHT_BLACK, BG_BLACK) : COLOR(FG_BLUE, BG_BLACK)) : COLOR(FG_YELLOW, BG_BLACK));
        SETCOLOR(FG_BRIGHT_WHITE, BG_BLACK);
        MOVETOT(29, 21);
        printf("%s-Home button", tui_enter_dfu_loading_progress > 14 ? (supports_bright_colors ? COLOR(FG_BRIGHT_BLACK, BG_BLACK) : COLOR(FG_BLUE, BG_BLACK)) : COLOR(FG_YELLOW, BG_BLACK));
        SETCOLOR(FG_BRIGHT_WHITE, BG_BLACK);

        break;
    }
    fflush(stdout);
}

// TODO: IMPLEMENT REAL ENTER RECOVERY
int dfu_time = 0;

tui_screen_t tui_enter_dfu(void) {
    if (tui_connected_devices && !tui_connected_devices->next) {
        if (tui_connected_devices->mode == TUI_DEVICE_MODE_NORMAL) {
            tui_enter_dfu_status = 0;
            return ENTER_RECOVERY_SCREEN;
        }
    }
    int i = 0;
    tui_enter_dfu_loading_progress = 1;
    while(1) {
        if (++i % 10 == 0) {
            tui_enter_dfu_loading_progress++;
        }
        redraw_screen();
        usleep(100000);
        int event = tui_try_get_event();
        if (event == TUI_EVENT_CONNECTED_DEVICES_CHANGED) {
            if (tui_connected_devices && !tui_connected_devices->next) {
                if (tui_connected_devices->mode == TUI_DEVICE_MODE_RECOVERY || tui_connected_devices->mode == TUI_DEVICE_MODE_NORMAL) {
                    tui_enter_dfu_status = 2;
                    tui_device_is_entering_dfu = false;
                    return ENTER_DFU_SCREEN;
                } else {
                    tui_enter_dfu_status = 1;
                    tui_device_is_entering_dfu = false;
                    tui_enter_dfu_loading_progress = 15;
                    redraw_screen();
                    sleep(3);
                    tui_jailbreak();
                    return JAILBREAK_SCREEN;
                }
            }
        }
        if (i > 10 * (60 + 15)) {
            return MAIN_SCREEN;
        }
        if (i == 10 * 2) {
            exitrecv_cmd(tui_connected_devices->ecid);
        }
    }
    redraw_screen();
    fflush(stdout);
    sleep(3);
    return JAILBREAK_SCREEN;
}

int tui_enter_dfu_nav_mouse_select = -1;

tui_screen_t tui_screen_enter_dfu(void) {
    if (tui_connected_devices && !tui_connected_devices->next) {
        tui_enter_dfu_device_type = tui_product_type_to_device_type(tui_connected_devices->product_type);
    }

    tui_device_is_entering_dfu = false;
    tui_enter_dfu_loading_progress = 0;
    tui_enter_dfu_nav_selection = 1;
    redraw_screen();
    
    tui_device_is_entering_dfu = false;
    tui_enter_dfu_loading_progress = 0;

    while(1) {
        int event = tui_get_event();
        switch (event) {
            case TUI_EVENT_CONNECTED_DEVICES_CHANGED:
                break;
            case TUI_EVENT_INPUT: {
                int input = tui_last_input;
                if (input == TUI_INPUT_NONE) continue;
                if (tui_device_is_entering_dfu) continue;

                switch (input) {
                    case TUI_INPUT_LEFT:
                    case TUI_INPUT_RIGHT:
                        tui_enter_dfu_nav_selection = !tui_enter_dfu_nav_selection;
                        redraw_screen();
                        break;
                    case TUI_INPUT_SELECT:
                        switch(tui_enter_dfu_nav_selection) {
                        case 0:
                            return MAIN_SCREEN;
                        case 1:
                            tui_device_is_entering_dfu = true;
                            redraw_screen();
                            return tui_enter_dfu();
                        }
                    case TUI_INPUT_MOUSE_MOVE:
                        tui_screen_enter_dfu_nav();
                        fflush(stdout);
                        break;
                    case TUI_INPUT_MOUSE_DOWN:
                        if (tui_mouse_y == tui_y_offset + 22) {
                            if (tui_mouse_x >= tui_x_offset + 80 - 23 && tui_mouse_x <= tui_x_offset + 80 - 23 + 10) {
                                tui_enter_dfu_nav_selection = 0;
                                tui_enter_dfu_nav_mouse_select = 0;
                                redraw_screen();
                            } else if (tui_mouse_x >= tui_x_offset + 80 - 12 && tui_mouse_x <= tui_x_offset + 80 - 12 + 10) {
                                tui_enter_dfu_nav_selection = 1;
                                tui_enter_dfu_nav_mouse_select = 1;
                                redraw_screen();
                            }
                        }
                        break;
                    case TUI_INPUT_MOUSE_UP:
                        if (tui_mouse_y == tui_y_offset + 22) {
                            if (tui_mouse_x >= tui_x_offset + 80 - 23 && tui_mouse_x <= tui_x_offset + 80 - 23 + 10) {
                                return MAIN_SCREEN;
                            } else if (tui_mouse_x >= tui_x_offset + 80 - 12 && tui_mouse_x <= tui_x_offset + 80 - 12 + 10) {
                                tui_device_is_entering_dfu = true;
                                redraw_screen();
                                return tui_enter_dfu();
                            }
                        }
                        tui_enter_dfu_nav_mouse_select = -1;
                        break;
                }
                break;
            }
        }
    }
    return MAIN_SCREEN;
}
#else
/* ISO C forbids an empty translation unit */
extern char** environ;
#endif

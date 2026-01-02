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

static int tui_options_nav_selection = 0;
static int tui_options_nav_mouse_select = 0;

static bool tui_options_allow_untested = false;
bool tui_options_safe_mode = false;
bool tui_options_verbose_boot = false;
bool tui_options_force_revert = false;
bool tui_options_flower_chain = true;

bool tui_options_is_editing_boot_args = false;

char tui_options_boot_args[0x1e0 + 1] = {0};

int tui_options_boot_args_cursor = 0;
int tui_options_boot_args_offset = 0;

void tui_screen_options_nav(void) {
    SETCOLOR(FG_WHITE, BG_BLACK);
    MOVETOT(80 - 11, 23);
    printf("%s[  Back  ]" COLOR(FG_WHITE, BG_BLACK),
        tui_options_nav_selection == 0 ? COLOR(FG_BLACK, BG_BRIGHT_WHITE) : (
            (
                tui_mouse_x >= tui_x_offset + 80 - 12 && tui_mouse_x <= tui_x_offset + 80 - 12 + 10
            ) && (tui_mouse_y == tui_y_offset + 22) ? COLOR(FG_BLACK, BG_WHITE) : ""
        )
    );
#ifdef DEV_BUILD
    PRINTATT(3, 23, "DEV_BUILD");
#endif
}

extern bool easter_egg;

void tui_screen_options_options(void) {
    SETCOLOR( FG_BRIGHT_BLACK, BG_BLACK);
    MOVETOT(3, 8);
    printf("[%c] %sAllow untested iOS/iPadOS versions" COLOR(FG_WHITE, BG_BLACK),
        tui_options_allow_untested ? ' ' : ' ',
        COLOR(FG_BLACK, FG_BRIGHT_BLACK));
    SETCOLOR(FG_WHITE, BG_BLACK);
    MOVETOT(3, 9);
    printf("[%c] %sSafe Mode                         " COLOR(FG_WHITE, BG_BLACK),
        tui_options_safe_mode ? 'x' : ' ',
        tui_options_nav_selection == 1 ? COLOR(FG_BLACK, BG_BRIGHT_WHITE) : (
            (
                tui_mouse_x >= tui_x_offset + 5 && tui_mouse_x <= tui_x_offset + 5 + 34
            ) && (tui_mouse_y == tui_y_offset + 8) ? COLOR(FG_BLACK, BG_WHITE) : ""
        ));
    MOVETOT(3, 10);
    printf("[%c] %sVerbose Boot                      " COLOR(FG_WHITE, BG_BLACK),
        tui_options_verbose_boot ? 'x' : ' ',
        tui_options_nav_selection == 2 ? COLOR(FG_BLACK, BG_BRIGHT_WHITE) : (
            (
                tui_mouse_x >= tui_x_offset + 5 && tui_mouse_x <= tui_x_offset + 5 + 34
            ) && (tui_mouse_y == tui_y_offset + 9) ? COLOR(FG_BLACK, BG_WHITE) : ""
        ));
    MOVETOT(3, 11);
    printf("[%c] %sForce Revert                      " COLOR(FG_WHITE, BG_BLACK),
        tui_options_force_revert ? 'x' : ' ',
        tui_options_nav_selection == 3 ? COLOR(FG_BLACK, BG_BRIGHT_WHITE) : (
            (
                tui_mouse_x >= tui_x_offset + 5 && tui_mouse_x <= tui_x_offset + 5 + 34
            ) && (tui_mouse_y == tui_y_offset + 10) ? COLOR(FG_BLACK, BG_WHITE) : ""
        ));
    if (supports_bright_colors) {
        SETCOLOR(FG_BRIGHT_BLACK, BG_BLACK);
    } else {
        SETCOLOR(FG_BLUE, BG_BLACK);
    }
    DRAWLINET(3, 13, 76);
    SETCOLOR(FG_WHITE, BG_BLACK);
    MOVETOT(7, 15);
    printf("%sBoot Arguments:                   " COLOR(FG_WHITE, BG_BLACK),
        tui_options_nav_selection == 4 ? COLOR(FG_BLACK, BG_BRIGHT_WHITE) : (
            (
                tui_mouse_x >= tui_x_offset + 5 && tui_mouse_x <= tui_x_offset + 5 + 34
            ) && (
                tui_mouse_y == tui_y_offset + 14 || tui_mouse_y == tui_y_offset + 15
            ) ? COLOR(FG_BLACK, BG_WHITE) : ""
        ));
    const char* boot_args_hint = tui_options_is_editing_boot_args ? "(press ENTER when done)" : "  (press ENTER to edit)";
    MOVETOT((int)(80 - 5 - strlen(boot_args_hint)), 15);
    printf("%s", boot_args_hint);
    MOVETOT(3, 17);
    printf("[%c] %s%s                      " COLOR(FG_WHITE, BG_BLACK),
        tui_options_flower_chain ? 'x' : ' ',
        tui_options_nav_selection == 5 ? COLOR(FG_BLACK, BG_BRIGHT_WHITE) : (
            (
                tui_mouse_x >= tui_x_offset + 5 && tui_mouse_x <= tui_x_offset + 5 + 34
            ) && (tui_mouse_y == tui_y_offset + 16) ? COLOR(FG_BLACK, BG_WHITE) : ""
        ), easter_egg ? "Neko Chain  " : "Flower Chain");

    MOVETOT(7, 16);
    if (supports_bright_colors) {
        SETCOLOR(tui_options_is_editing_boot_args ? FG_YELLOW : FG_WHITE, BG_BRIGHT_BLACK);
    } else {
        SETCOLOR(tui_options_is_editing_boot_args ? FG_YELLOW : FG_WHITE, BG_BLUE);
    }
    int boot_args_len = strlen(tui_options_boot_args);

    if ((tui_options_boot_args_cursor - tui_options_boot_args_offset) > (68 - 1)) {
        tui_options_boot_args_offset += (tui_options_boot_args_cursor - tui_options_boot_args_offset) - (68 - 1);
    }
    if ((tui_options_boot_args_cursor - tui_options_boot_args_offset) < 0) {
        tui_options_boot_args_offset = tui_options_boot_args_cursor;
    }
    if ((tui_options_boot_args_offset + 68 - 1) > boot_args_len && boot_args_len >= 68 - 1) {
        tui_options_boot_args_offset = boot_args_len - 68 + 1;
    }

    for (int i = 0; i < 68; i++) {
        putchar(((i + tui_options_boot_args_offset) < boot_args_len) ? tui_options_boot_args[i + tui_options_boot_args_offset] : ' ');
    }

    if (tui_options_is_editing_boot_args) {
        MOVETOT((int)(7 + tui_options_boot_args_cursor - tui_options_boot_args_offset), 16);
        if (supports_bright_colors) {
            SETCOLOR(FG_BRIGHT_BLACK, BG_YELLOW);
        } else {
            SETCOLOR(FG_BLUE, BG_YELLOW);
        }
        putchar(tui_options_boot_args_cursor < boot_args_len ? tui_options_boot_args[tui_options_boot_args_cursor] : ' ');
    }

    SETCOLOR(FG_WHITE, BG_BLACK);
    MOVETOT(5, 16);
    printf(tui_options_boot_args_offset > 0 ? "<<" : "  ");
    MOVETOT(75, 16);
    printf((strlen(tui_options_boot_args) - tui_options_boot_args_offset) > 68 ? ">>" : "  ");
}

void tui_screen_options_redraw(void) {
    SETCOLOR(FG_WHITE, BG_BLACK);
    PRINTATT(3, 3, "You may set the following options. If you don't know what they mean you'll");
    PRINTATT(3, 4, "probably have no reason to set them.");
    if (supports_bright_colors) {
        SETCOLOR(FG_BRIGHT_BLACK, BG_BLACK);
    } else {
        SETCOLOR(FG_BLUE, BG_BLACK);
    }
    DRAWLINET(3, 6, 76);
    tui_screen_options_nav();
    tui_screen_options_options();
    fflush(stdout);
}

tui_screen_t tui_screen_options(void) {
    redraw_screen();

    while(1) {
        int event = tui_get_event();
        if (event != TUI_EVENT_INPUT) continue;
        int input = tui_last_input;

        switch (input) {
            case TUI_INPUT_DOWN:
            case TUI_INPUT_TAB:
                if (tui_options_is_editing_boot_args) break;
                if (++tui_options_nav_selection > 5) tui_options_nav_selection = 0;
                tui_screen_options_nav();
                tui_screen_options_options();
                fflush(stdout);
                break;
            case TUI_INPUT_UP:
            case TUI_INPUT_TAB_BACK:
                if (tui_options_is_editing_boot_args) break;
                if (--tui_options_nav_selection < 0) tui_options_nav_selection = 5;
                tui_screen_options_nav();
                tui_screen_options_options();
                fflush(stdout);
                break;
            case TUI_INPUT_SELECT:
                if (tui_options_is_editing_boot_args && tui_last_key == ' ' && tui_options_boot_args[strlen(tui_options_boot_args)] == '\x00') {
                    size_t max_len = sizeof(tui_options_boot_args) - 1;
                    size_t len = strlen(tui_options_boot_args);
                    if (len >= max_len) {
                        break;
                    }
                    tui_options_boot_args[len] = ' ';
                    tui_options_boot_args[len + 1] = '\x00';
                    tui_options_boot_args_cursor++;
                    tui_screen_options_options();
                    fflush(stdout);
                    break;
                }
                switch(tui_options_nav_selection) {
                case 0:
                    return MAIN_SCREEN;
                // case 1:
                //     tui_options_allow_untested = !tui_options_allow_untested;
                //     break;
                case 1:
                    tui_options_safe_mode = !tui_options_safe_mode;
                    break;
                case 2:
                    tui_options_verbose_boot = !tui_options_verbose_boot;
                    break;
                case 3:
                    tui_options_force_revert = !tui_options_force_revert;
                    break;
                case 4:
                    tui_options_is_editing_boot_args = !tui_options_is_editing_boot_args;
                    tui_options_boot_args_cursor = tui_options_is_editing_boot_args ? strlen(tui_options_boot_args) : 0;
                    break;
                case 5:
                    tui_options_flower_chain = !tui_options_flower_chain;
                    break;
                }
                tui_screen_options_nav();
                tui_screen_options_options();
                fflush(stdout);
                break;
            case TUI_INPUT_MOUSE_MOVE:
                tui_screen_options_nav();
                tui_screen_options_options();
                fflush(stdout);
                break;
            case TUI_INPUT_MOUSE_DOWN:
                if (tui_mouse_y == tui_y_offset + 24 - 2 &&
                    tui_mouse_x >= tui_x_offset + 80 - 12 &&
                    tui_mouse_x <= tui_x_offset + 80 - 12 + 10) {
                    tui_options_nav_selection = 0;
                    tui_options_nav_mouse_select = 0;
                    redraw_screen();
                }

#define MOUSEDOWN_HANDLER(y, val)                               \
                if (tui_mouse_y == tui_y_offset + y &&          \
                    tui_mouse_x >= tui_x_offset + 5 - 4 &&      \
                    tui_mouse_x <= tui_x_offset + 5 + 34) {     \
                    tui_options_nav_selection = val;            \
                    tui_options_nav_mouse_select = val;         \
                    tui_screen_options_nav();                   \
                    tui_screen_options_options();               \
                    fflush(stdout);                             \
                }

                if (!tui_options_is_editing_boot_args) {
                    MOUSEDOWN_HANDLER(8, 1);
                    MOUSEDOWN_HANDLER(9, 2);
                    MOUSEDOWN_HANDLER(10, 3);
                    MOUSEDOWN_HANDLER(16, 5);
                }
                
                MOUSEDOWN_HANDLER(14, 4);
                MOUSEDOWN_HANDLER(15, 4);

#undef MOUSEDOWN_HANDLER
                break;
            case TUI_INPUT_MOUSE_UP:
                if (tui_mouse_y == tui_y_offset + 24 - 2 &&
                    tui_mouse_x >= tui_x_offset + 80 - 12 &&
                    tui_mouse_x <= tui_x_offset + 80 - 12 + 10 &&
                    tui_options_nav_mouse_select == 0) {
                    return MAIN_SCREEN;
                }

#define MOUSEUP_HANDLER(y, val, option, e)                          \
                if (tui_mouse_y == tui_y_offset + y &&              \
                    tui_mouse_x >= tui_x_offset + 5 - 4 &&          \
                    tui_mouse_x <= tui_x_offset + 5 + 34 &&         \
                    tui_options_nav_mouse_select == val) {          \
                    tui_options_##option = !tui_options_##option;   \
                    e                                               \
                    tui_screen_options_nav();                       \
                    tui_screen_options_options();                   \
                    fflush(stdout);                                 \
                }

                if (!tui_options_is_editing_boot_args) {
                    MOUSEUP_HANDLER(8, 1, safe_mode, );
                    MOUSEUP_HANDLER(9, 2, verbose_boot, );
                    MOUSEUP_HANDLER(10, 3, force_revert, );
                    MOUSEUP_HANDLER(16, 5, flower_chain, );
                }

                #define BOOTARGS_HANDLER                                                                                \
                tui_options_boot_args_cursor = tui_options_is_editing_boot_args ? strlen(tui_options_boot_args) : 0;

                MOUSEUP_HANDLER(14, 4, is_editing_boot_args, BOOTARGS_HANDLER);
                MOUSEUP_HANDLER(15, 4, is_editing_boot_args, BOOTARGS_HANDLER);
#undef MOUSEUP_HANDLER
                tui_options_nav_mouse_select = -1;
                break;
            case TUI_INPUT_BACKSPACE:
                if (tui_options_is_editing_boot_args && tui_options_boot_args_cursor > 0) {
                    memmove(tui_options_boot_args + tui_options_boot_args_cursor - 1, tui_options_boot_args + tui_options_boot_args_cursor, strlen(tui_options_boot_args) - tui_options_boot_args_cursor);
                    tui_options_boot_args[strlen(tui_options_boot_args) - 1] = '\x00';
                    tui_options_boot_args_cursor--;
                    tui_screen_options_options();
                    fflush(stdout);
                }
                break;
            case TUI_INPUT_NONE:
                if (tui_options_is_editing_boot_args && tui_options_boot_args[strlen(tui_options_boot_args)] == '\x00') {
                    size_t max_len = sizeof(tui_options_boot_args) - 1;
                    size_t len = strlen(tui_options_boot_args);
                    if (len >= max_len) {
                        break;
                    }
                    memmove(tui_options_boot_args + tui_options_boot_args_cursor + 1, tui_options_boot_args + tui_options_boot_args_cursor, strlen(tui_options_boot_args) - tui_options_boot_args_cursor);
                    tui_options_boot_args[tui_options_boot_args_cursor] = tui_last_key;
                    tui_options_boot_args[len + 1] = '\x00';
                    tui_options_boot_args_cursor++;
                    tui_screen_options_options();
                    fflush(stdout);
                }
                break;
            case TUI_INPUT_LEFT:
                if (tui_options_is_editing_boot_args) {
                    if (tui_options_boot_args_cursor > 0) {
                        tui_options_boot_args_cursor--;
                        tui_screen_options_options();
                        fflush(stdout);
                    }
                }
                break;
            case TUI_INPUT_RIGHT:
                if (tui_options_is_editing_boot_args) {
                    if (tui_options_boot_args_cursor < (int)strlen(tui_options_boot_args)) {
                        tui_options_boot_args_cursor++;
                        tui_screen_options_options();
                        fflush(stdout);
                    }
                }
                break;
        }
    }
}

#else
/* ISO C forbids an empty translation unit */
extern char** environ;
#endif

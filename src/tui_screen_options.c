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

int tui_options_nav_selection = 0;
int tui_options_nav_mouse_select = 0;

bool tui_options_allow_untested = false;
bool tui_options_safe_mode = false;
bool tui_options_verbose_boot = false;
bool tui_options_force_revert = false;
bool tui_options_flower_chain = true;

bool tui_options_is_editing_boot_args = false;

char tui_options_boot_args[0x1e0 + 1] = {0};

unsigned tui_options_boot_args_cursor = 0;

void tui_screen_options_nav(void) {
    SETCOLOR(FG_WHITE, BG_BLACK);
    MOVETOT(80 - 11, 23);
    printf("%s[  Back  ]\033[37;40m",
        tui_options_nav_selection == 0 ? "\033[30;107m" : (
            (
                tui_mouse_x >= tui_x_offset + 80 - 12 && tui_mouse_x <= tui_x_offset + 80 - 12 + 10
            ) && (tui_mouse_y == tui_y_offset + 22) ? "\x1b[30;47m" : ""
        )
    );
#ifdef DEV_BUILD
    PRINTATT(3, 23, "DEV_BUILD");
#endif
}

extern bool easter_egg;

void tui_screen_options_options(void) {
    SETCOLOR(FG_WHITE, BG_BLACK);
    MOVETOT(3, 8);
    printf("[%c] %sAllow untested iOS/iPadOS versions\x1b[37;40m",
        tui_options_allow_untested ? 'x' : ' ',
        tui_options_nav_selection == 1 ? "\x1b[30;107m" : (
            (
                tui_mouse_x >= tui_x_offset + 6 && tui_mouse_x <= tui_x_offset + 6 + 34
            ) && (tui_mouse_y == tui_y_offset + 7) ? "\x1b[30;47m" : ""
        ));
    MOVETOT(3, 9);
    printf("[%c] %sSafe Mode                         \x1b[37;40m",
        tui_options_safe_mode ? 'x' : ' ',
        tui_options_nav_selection == 2 ? "\x1b[30;107m" : (
            (
                tui_mouse_x >= tui_x_offset + 6 && tui_mouse_x <= tui_x_offset + 6 + 34
            ) && (tui_mouse_y == tui_y_offset + 8) ? "\x1b[30;47m" : ""
        ));
    MOVETOT(3, 10);
    printf("[%c] %sVerbose Boot                      \x1b[37;40m",
        tui_options_verbose_boot ? 'x' : ' ',
        tui_options_nav_selection == 3 ? "\x1b[30;107m" : (
            (
                tui_mouse_x >= tui_x_offset + 6 && tui_mouse_x <= tui_x_offset + 6 + 34
            ) && (tui_mouse_y == tui_y_offset + 9) ? "\x1b[30;47m" : ""
        ));
    MOVETOT(3, 11);
    printf("[%c] %sForce Revert                      \x1b[37;40m",
        tui_options_force_revert ? 'x' : ' ',
        tui_options_nav_selection == 4 ? "\x1b[30;107m" : (
            (
                tui_mouse_x >= tui_x_offset + 6 && tui_mouse_x <= tui_x_offset + 6 + 34
            ) && (tui_mouse_y == tui_y_offset + 10) ? "\x1b[30;47m" : ""
        ));
    SETCOLOR(FG_BRIGHT_BLACK, BG_BLACK);
    DRAWLINET(3, 13, 76);
    SETCOLOR(FG_WHITE, BG_BLACK);
    MOVETOT(7, 15);
    printf("%sBoot Arguments:                   \x1b[37;40m",
        tui_options_nav_selection == 5 ? "\x1b[30;107m" : (
            (
                tui_mouse_x >= tui_x_offset + 6 && tui_mouse_x <= tui_x_offset + 6 + 34
            ) && (
                tui_mouse_y == tui_y_offset + 14 || tui_mouse_y == tui_y_offset + 15
            ) ? "\x1b[30;47m" : ""
        ));
    const char* boot_args_hint = tui_options_is_editing_boot_args ? "(press ENTER when done)" : "  (press ENTER to edit)";
    MOVETOT((int)(80 - 5 - strlen(boot_args_hint)), 15);
    printf("%s", boot_args_hint);
    MOVETOT(3, 17);
    printf("[%c] %s%s                      \x1b[37;40m",
        tui_options_flower_chain ? 'x' : ' ',
        tui_options_nav_selection == 6 ? "\x1b[30;107m" : (
            (
                tui_mouse_x >= tui_x_offset + 6 && tui_mouse_x <= tui_x_offset + 6 + 34
            ) && (tui_mouse_y == tui_y_offset + 16) ? "\x1b[30;47m" : ""
        ), easter_egg ? "Neko Chain  " : "Flower Chain");

    SETCOLOR(FG_WHITE, BG_BLACK);
    if (strlen(tui_options_boot_args) > 68) {
        MOVETOT(75, 16);
        printf(">>");
    }

    MOVETOT(7, 16);
    SETCOLOR(tui_options_is_editing_boot_args ? FG_YELLOW : FG_WHITE, BG_BRIGHT_BLACK);

    unsigned boot_args_len = strlen(tui_options_boot_args);

    for (unsigned i = 0; i < 68; i++) {
        putchar(i < boot_args_len ? tui_options_boot_args[i] : ' ');
    }

    if (tui_options_is_editing_boot_args) {
        MOVETOT((int)(7 + tui_options_boot_args_cursor), 16);
        SETCOLOR(FG_BRIGHT_BLACK, BG_YELLOW);
        putchar(tui_options_boot_args_cursor < boot_args_len ? tui_options_boot_args[tui_options_boot_args_cursor] : ' ');
    }
}

void tui_screen_options_redraw(void) {
    SETCOLOR(FG_WHITE, BG_BLACK);
    PRINTATT(3, 3, "You may set the following options. If you don't know what they mean you'll");
    PRINTATT(3, 4, "probably have no reason to set them.");
    SETCOLOR(FG_BRIGHT_BLACK, BG_BLACK);
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
                if (++tui_options_nav_selection > 6) tui_options_nav_selection = 0;
                tui_screen_options_nav();
                tui_screen_options_options();
                fflush(stdout);
                break;
            case TUI_INPUT_UP:
            case TUI_INPUT_TAB_BACK:
                if (tui_options_is_editing_boot_args) break;
                if (--tui_options_nav_selection < 0) tui_options_nav_selection = 6;
                tui_screen_options_nav();
                tui_screen_options_options();
                fflush(stdout);
                break;
            case TUI_INPUT_SELECT:
                if (tui_options_is_editing_boot_args && tui_last_key == ' ' && tui_options_boot_args[strlen(tui_options_boot_args)] == '\x00') {
                    tui_options_boot_args[strlen(tui_options_boot_args)] = ' ';
                    tui_screen_options_options();
                    fflush(stdout);
                    break;
                }
                switch(tui_options_nav_selection) {
                case 0:
                    return MAIN_SCREEN;
                case 1:
                    tui_options_allow_untested = !tui_options_allow_untested;
                    break;
                case 2:
                    tui_options_safe_mode = !tui_options_safe_mode;
                    break;
                case 3:
                    tui_options_verbose_boot = !tui_options_verbose_boot;
                    break;
                case 4:
                    tui_options_force_revert = !tui_options_force_revert;
                    break;
                case 5:
                    tui_options_is_editing_boot_args = !tui_options_is_editing_boot_args;
                    tui_options_boot_args_cursor = strlen(tui_options_boot_args);
                    break;
                case 6:
                    tui_options_flower_chain = !tui_options_flower_chain;
                    break;
                }
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
                    tui_mouse_x >= tui_x_offset + 6 - 4 &&      \
                    tui_mouse_x <= tui_x_offset + 6 + 34) {     \
                    tui_options_nav_selection = val;            \
                    tui_options_nav_mouse_select = val;         \
                    tui_screen_options_nav();                   \
                    tui_screen_options_options();               \
                    fflush(stdout);                             \
                }

                if (!tui_options_is_editing_boot_args) {
                    MOUSEDOWN_HANDLER(7, 1);
                    MOUSEDOWN_HANDLER(8, 2);
                    MOUSEDOWN_HANDLER(9, 3);
                    MOUSEDOWN_HANDLER(10, 4);
                    MOUSEDOWN_HANDLER(16, 6);
                }
                
                MOUSEDOWN_HANDLER(14, 5);
                MOUSEDOWN_HANDLER(15, 5);

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
                    tui_mouse_x >= tui_x_offset + 6 - 4 &&          \
                    tui_mouse_x <= tui_x_offset + 6 + 34 &&         \
                    tui_options_nav_mouse_select == val) {          \
                    tui_options_##option = !tui_options_##option;   \
                    e;                                              \
                    tui_screen_options_nav();                       \
                    tui_screen_options_options();                   \
                    fflush(stdout);                                 \
                }

                if (!tui_options_is_editing_boot_args) {
                    MOUSEUP_HANDLER(7, 1, allow_untested, );
                    MOUSEUP_HANDLER(8, 2, safe_mode, );
                    MOUSEUP_HANDLER(9, 3, verbose_boot, );
                    MOUSEUP_HANDLER(10, 4, force_revert, );
                    MOUSEUP_HANDLER(16, 6, flower_chain, );
                }

                MOUSEUP_HANDLER(14, 5, is_editing_boot_args, tui_options_boot_args_cursor = strlen(tui_options_boot_args));
                MOUSEUP_HANDLER(15, 5, is_editing_boot_args, tui_options_boot_args_cursor = strlen(tui_options_boot_args));
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
                    int len = strlen(tui_options_boot_args);
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
                    if (tui_options_boot_args_cursor < strlen(tui_options_boot_args)) {
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

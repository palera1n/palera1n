#define NCURSES_WIDECHAR 1

#include <ncurses.h>
#include <form.h>
#include <menu.h>
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

#include <common.h>
#include <tui.h>

int redraw_screen() {
    erase();
    resizeterm(24, 80);
    box(stdscr, 0, 0);
    mvaddstr(0, 80 - strlen("[palera1n version " PALERAIN_VERSION "]") - 2 , "[palera1n version " PALERAIN_VERSION "]");
    return 0;
}

int destroy_window() {
    endwin();
    return 0;
}

int init_window() {
    setlocale(LC_ALL, NULL);
    initscr();
    if (has_colors() == TRUE) {
        start_color();
        init_pair(MSG_COLOR, COLOR_YELLOW, COLOR_BLACK);
        init_pair(ICON_COLOR, COLOR_WHITE, COLOR_BLACK);
    }
    int height, width;
    getmaxyx(stdscr, height, width);
    if (width < 80 || height < 24) {
        LOG(LOG_ERROR, "Terminal size must be at least 80x24");
        return -1;
    }
    if (has_colors() == FALSE) {
        LOG(LOG_VERBOSE2, "Terminal without color support detected");
    }
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    return 0;
}

int tui() {
    int ret = MAIN_SCREEN;
    init_window();
    while (1) {
        redraw_screen();
        switch (ret) {
            case ERROR_SCREEN:
                goto out;
                break;
            case EXIT_SCREEN:
                goto out;
                break;
            case MAIN_SCREEN:
                ret = tui_screen_main();
                break;
            case OPTIONS_SCREEN:
                ret = tui_screen_options();
                break;
            case ENTER_RECOVERY_SCREEN:
                ret = tui_screen_enter_recovery();
                break;
            case ENTER_DFU_SCREEN:
                ret = tui_screen_enter_dfu();
                break;
            case JAILBREAKING_SCREEN:
                ret = tui_screen_jailbreak();
                break;
            default:
                assert(0);
                goto out;
        }
    }
out:
    destroy_window();
    return ret;
}

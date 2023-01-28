#define NCURSES_WIDECHAR 1

#include <ncursestw/ncurses.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <locale.h>
#include <unistd.h>

#include "common.h"
#include "tui.h"

#define MSG_COLOR 1
#define ICON_COLOR 2
#define COLOR_GREY init_color(0, 300, 300, 300)

struct button_list main_menu_buttons = {
    .count = 3,
    .current = 2,
    .height = 22,
    .buttons = {
        { .name = "Exit" },
        { .name = "Start" },
        { .name = "Options "},
    },
};

int show_buttons(struct button_list* list) {
    uint8_t start = 78;
    char button_buf[0x20];
    for (uint8_t i = 0; i < list->count; i++) {
        start -= (strlen(list->buttons[i].name) + 5);
        snprintf(button_buf, 0x20, "[ %s ] ", list->buttons[i].name);
        mvaddstr(list->height, start, button_buf);
    }
    return 0;
}

int main_menu() {
    mvaddstr(1, 2, "Welcome to palera1n!");
    move(2,1);
    whline(stdscr, ACS_HLINE, 80 - 2);
    if (has_colors() == TRUE) {
        start_color();
        init_pair(MSG_COLOR, COLOR_YELLOW, COLOR_BLACK);
        init_pair(ICON_COLOR, COLOR_GREY, COLOR_BLACK);
    }
    move(5,2);
    attron(COLOR_PAIR(MSG_COLOR));
    mvaddstr(3,2, "Connect your iPhone, iPad, or iPod Touch");
    mvaddstr(4,2, "to begin.");
    attroff(COLOR_PAIR(MSG_COLOR));
    attron(COLOR_PAIR(ICON_COLOR));
    mvaddwstr(5,  43, L"MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM");
    mvaddwstr(6,  43, L"MMMMMMMMMMMMMWNOddONWMMMMMMMMMMMMM");
    mvaddwstr(7,  43, L"MMMMMMMMMMMMWNx.  .xNWMMMMMMMMMMMM");
    mvaddwstr(8,  43, L"MMMMMMMMMMMMW0;....;OWMMMMMMMMMMMM");
    mvaddwstr(9,  43, L"MMMMMMMMMMMMW0;....;OWMMMMMMMMMMMM");
    mvaddwstr(10, 43, L"MMMMMMMMMMMWXo'.....lKWMMMMMMMMMMM");
    mvaddwstr(11, 43, L"MMMMMMMMMMWNkc;'....'dNWMMMMMMMMMM");
    mvaddwstr(12, 43, L"MMMMMMMMMMN0dc:,''''.:ONWMMMMMMMMM");
    mvaddwstr(13, 43, L"MMMMMMMMMWKxllc;,'''',lKWMMMMMMMMM");
    mvaddwstr(14, 43, L"MMMMMMMMWNkoolc;,,,,;;:xXWMMMMMMMM");
    mvaddwstr(15, 43, L"MMMMMMMMW0olllc;,,,,,,,:OWMMMMMMMM");
    mvaddwstr(16, 43, L"MMMMMMMMNOollc:,'....'';xNMMMMMMMM");
    mvaddwstr(17, 43, L"MMMMMMMMW0dllc:,''''''':OWMMMMMMMM");
    mvaddwstr(18, 43, L"MMMMMMMMWN0dc::;;,,,,,cONWMMMMMMMM");
    mvaddwstr(19, 43, L"MMMMMMMMMMWXOxoc::cldkXWMMMMMMMMMM");
    mvaddwstr(20, 43, L"MMMMMMMMMMMMWWNNXXNNWWMMMMMMMMMMMM");
    mvaddwstr(21, 43, L"MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM");
    attroff(COLOR_PAIR(ICON_COLOR));
    move(21,1);
    whline(stdscr, ACS_HLINE, 80 - 2);
    show_buttons(&main_menu_buttons);
    refresh();
    return 0;
}

int init_window() {
    setlocale(LC_ALL, NULL);
    initscr();
    int height, width;
    getmaxyx(stdscr, height, width);
    if (width < 80 || height < 24) {
        LOG(LOG_ERROR, "Terminal size must be at least 80x24");
        return -1;
    }
    if (has_colors() == FALSE) {
        LOG(LOG_VERBOSE2, "Terminal without color support detected");
    }
    resizeterm(24, 80);
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    box(stdscr, 0, 0);
    mvaddstr(0, 80 - strlen("[ palera1n version " PALERAIN_VERSION " ]") - 3 , "[ palera1n version " PALERAIN_VERSION " ]");
    return 0;
}

int destroy_window() {
    endwin();
    return 0;
}

int tui() {
    int ret;
    ret = init_window();
    main_menu();
    sleep(5);
    destroy_window();
    return ret;
}

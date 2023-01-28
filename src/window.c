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

#define MSG_COLOR 1
#define ICON_COLOR 2
#define BUTTON_COLOR_ON 3
#define COLOR_GREY init_color(30, 300, 300, 300)

ITEM* main_buttons[4];
MENU* main_menu;

int display_main_menu() {
    int ret = -1;
    mvaddstr(1, 2, "Welcome to palera1n!");
    move(2,1);
    whline(stdscr, ACS_HLINE, 80 - 2);
    move(5,2);
    attron(COLOR_PAIR(MSG_COLOR));
    mvaddstr(4,2, "Connect your iPhone, iPad, or iPod Touch");
    mvaddstr(5,2, "to begin.");
    attroff(COLOR_PAIR(MSG_COLOR));
    attron(COLOR_PAIR(ICON_COLOR));
    mvaddwstr(2, 43, L" MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM ");
    mvaddwstr(3, 44,  L"MMMMMMMMMMMMMWNOddONWMMMMMMMMMMMMM");
    mvaddwstr(4,  44, L"MMMMMMMMMMMMWNx.  .xNWMMMMMMMMMMMM");
    mvaddwstr(5,  44, L"MMMMMMMMMMMMW0;....;OWMMMMMMMMMMMM");
    mvaddwstr(6,  44, L"MMMMMMMMMMMMW0;....;OWMMMMMMMMMMMM");
    mvaddwstr(7,  44, L"MMMMMMMMMMMWXo'.....lKWMMMMMMMMMMM");
    mvaddwstr(8,  44, L"MMMMMMMMMMWNkc;'....'dNWMMMMMMMMMM");
    mvaddwstr(9,  44, L"MMMMMMMMMMN0dc:,''''.:ONWMMMMMMMMM");
    mvaddwstr(10, 44, L"MMMMMMMMMWKxllc;,'''',lKWMMMMMMMMM");
    mvaddwstr(11, 44, L"MMMMMMMMWNkoolc;,,,,;;:xXWMMMMMMMM");
    mvaddwstr(12, 44, L"MMMMMMMMW0olllc;,,,,,,,:OWMMMMMMMM");
    mvaddwstr(13, 44, L"MMMMMMMMNOollc:,'....'';xNMMMMMMMM");
    mvaddwstr(14, 44, L"MMMMMMMMW0dllc:,''''''':OWMMMMMMMM");
    mvaddwstr(15, 44, L"MMMMMMMMWN0dc::;;,,,,,cONWMMMMMMMM");
    mvaddwstr(16, 44, L"MMMMMMMMMMWXOxoc::cldkXWMMMMMMMMMM");
    mvaddwstr(17, 44, L"MMMMMMMMMMMMWWNNXXNNWWMMMMMMMMMMMM");
    mvaddwstr(18, 44, L"MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM");
    mvaddstr(7, 2, "Made by: Nick Chan, Mineek, Tom, Nebula,");
    mvaddstr(8, 2, "llsc12, Nathan");
    move(9, 2);
    whline(stdscr, ACS_HLINE, 40);
    mvaddstr(10, 2, "Thanks to: nyuszika7h, Procursus Team");
    mvaddstr(11, 2, "dora2ios, m1sta, Serena, xerub, tihmstar");
    mvaddstr(12, 2, "sbingner, checkra1n team (Siguza, Axi0mx");
    mvaddstr(13, 2, "argp et al.)");
    move(14, 2);
    whline(stdscr, ACS_HLINE, 40);
    mvaddstr(19, 2, "palera1n is NOT affinated with checkra1n");
    mvaddstr(20, 2, "Note: Backup your stuff. Use at your own risk.");  
    mvaddstr(19, 78 - strlen("@palera1n"), "@palera1n");
    mvaddstr(20, 78 - strlen("https://palera.in"), "https://palera.in");
    attroff(COLOR_PAIR(ICON_COLOR));
    move(21,1);
    whline(stdscr, ACS_HLINE, 80 - 2);
    move(22, 40);

    main_buttons[0] = new_item(MAIN_OPTIONS, "");
    main_buttons[1] = new_item(MAIN_START, "");
    main_buttons[2] = new_item(MAIN_EXIT, "");
    main_buttons[3] = NULL;
    main_menu = new_menu(main_buttons);
    item_opts_off(main_buttons[1], O_SELECTABLE);
    set_menu_format(main_menu, 1, 40);
    set_menu_sub(main_menu, derwin(stdscr, 0, 0, 22, 50));
    post_menu(main_menu);
    raw();
    while (1) {
        uint8_t target = 0;
        int ch = getch();
        if (ch == KEY_LEFT) {
            menu_driver(main_menu, REQ_LEFT_ITEM);
        } else if (ch == KEY_RIGHT) {
            menu_driver(main_menu, REQ_RIGHT_ITEM);
        } else if (ch == 10) {
            if (!(item_opts(current_item(main_menu)) & O_SELECTABLE)) continue;
            // fprintf(stderr, "Button %s pressed\n", current_item(main_menu)->name.str);
            if (!strcmp(current_item(main_menu)->name.str, MAIN_EXIT)) {
                ret = EXIT_SCREEN;
            } else if (!strcmp(current_item(main_menu)->name.str, MAIN_START)) {
                ret = ENTER_RECOVERY_SCREEN;
            } else if (!strcmp(current_item(main_menu)->name.str, MAIN_OPTIONS)) {
                ret = OPTIONS_SCREEN;
            }
            goto end;
        }
        refresh();
    }
end:
    noraw();
    refresh();
    return ret;
}

int display_enter_dfu_screen() { return JAILBREAKING_SCREEN; }
int display_enter_recovery_screen() { return ENTER_DFU_SCREEN; }
int display_options_screen() { return MAIN_SCREEN; }
int display_jailbreaking_screen() { return EXIT_SCREEN; }

int init_window() {
    setlocale(LC_ALL, NULL);
    initscr();
    if (has_colors() == TRUE) {
        start_color();
        init_pair(MSG_COLOR, COLOR_YELLOW, COLOR_BLACK);
        init_pair(ICON_COLOR, COLOR_WHITE, COLOR_BLACK);
        init_pair(BUTTON_COLOR_ON, COLOR_BLACK, COLOR_WHITE);
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
    resizeterm(24, 80);
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    box(stdscr, 0, 0);
    mvaddstr(0, 80 - strlen("[palera1n version " PALERAIN_VERSION "]") - 2 , "[palera1n version " PALERAIN_VERSION "]");
    return 0;
}

int destroy_window() {
    endwin();
    return 0;
}

int tui() {
    int ret = MAIN_SCREEN;
    init_window();
    while (1) {
        switch (ret) {
            case ERROR_SCREEN:
                goto out;
                break;
            case EXIT_SCREEN:
                goto out;
                break;
            case MAIN_SCREEN:
                ret = display_main_menu();
                break;
            case OPTIONS_SCREEN:
                ret = display_options_screen();
                break;
            case ENTER_RECOVERY_SCREEN:
                ret = display_enter_recovery_screen();
                break;
            case ENTER_DFU_SCREEN:
                ret = display_enter_dfu_screen();
                break;
            case JAILBREAKING_SCREEN:
                ret = display_jailbreaking_screen();
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

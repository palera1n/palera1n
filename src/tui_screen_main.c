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

tui_screen_t tui_screen_main() {
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

    ITEM* main_buttons[4];
    MENU* main_menu;

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
    refresh();
    while (1) {
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
    unpost_menu(main_menu);
    free_menu(main_menu);
    free_item(main_buttons[0]);
    free_item(main_buttons[1]);
    free_item(main_buttons[2]);
    return ret;
}

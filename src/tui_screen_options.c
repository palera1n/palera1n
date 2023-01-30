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

tui_screen_t tui_screen_options() {
    tui_screen_t ret = EXIT_SCREEN;
    ITEM* back_button[2];
    MENU* back_menu;
    back_button[0] = new_item("Back", "");
    back_button[1] = NULL;
    back_menu = new_menu(back_button);

    move(21, 1);
    hline(ACS_HLINE, 80 - 2);

    set_menu_format(back_menu, 1, 10);
    set_menu_sub(back_menu, derwin(stdscr, 0, 0, 22, 68));
    post_menu(back_menu);

    FORM* options_form;
    FIELD* options[3];
    

    refresh();

    while(1) {
        int ch = getch();
        if (ch == 10) {
            if (!(item_opts(current_item(back_menu)) & O_SELECTABLE)) continue;
            if (!strcmp(current_item(back_menu)->name.str, BACK_BUTTON)) {
                ret = MAIN_SCREEN;
            }
            goto end;
        }
        refresh();
    }
end:
    unpost_menu(back_menu);
    free_item(back_button[0]);
    free_menu(back_menu);
    return ret;
}

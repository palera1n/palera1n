#ifdef WITH_TUI

#include "TuiSettingsPanel.hpp"
#include "Tui.hpp"

#include <ncurses.h>
#include <cstdio>
#include <cstring>

#include "../globals.h"
#include "../paleinfo.h"

static const char *buttons[] = { "[ Back ]" };

TuiSettingsPanel::TuiSettingsPanel(TuiFrame* frame)
    : TuiPanel(frame) {}

const char** TuiSettingsPanel::get_buttons() const {
    return buttons;
}

void TuiSettingsPanel::draw(int sy, int sx, int selected) {
    mvprintw(sy + 2, sx + 2, "You may set the following options. If you don't know what they mean you'll");
    mvprintw(sy + 3, sx + 2, "probably have no reason to set them.");

    int states[] = {
        !!(palerain_flags & palerain_option_safemode),
        !!(palerain_flags & palerain_option_verbose_boot),
        !!(palerain_flags & palerain_option_force_revert),
        !!(palerain_flags & palerain_option_flower_chain)
    };

    const char *labels[] = {
        "Safe Mode",
        "Verbose Boot",
        "Restore System",
        "Dark Blockchain"
    };

    for (int i = 0; i < 2; i++) {
        mvprintw(sy + 6 + i, sx + 2, "[%c] ", states[i] ? 'x' : ' ');
        if (selected == i) attron(A_REVERSE);
        printw("%s", labels[i]);
        if (selected == i) attroff(A_REVERSE);
    }

    mvprintw(sy + 8, sx + 6, "Boot Arguments:");
    if (selected == 2) attron(A_REVERSE);
    mvprintw(sy + 9, sx + 6, "%-66s", boot_args);
    if (selected == 2) attroff(A_REVERSE);

    for (int i = 2; i < 4; i++) {
        mvprintw(sy + 8 + i, sx + 2, "[%c] ", states[i] ? 'x' : ' ');
        if (selected == i + 1) attron(A_REVERSE);
        printw("%s", labels[i]);
        if (selected == i + 1) attroff(A_REVERSE);
    }
}

void TuiSettingsPanel::edit_boot_args(int start_y, int start_x) {
    curs_set(1);
    timeout(-1);

    int field_y = start_y + 9;
    int field_x = start_x + 6;
    int max_len = 51;

    char temp_buf[128];
    std::snprintf(temp_buf, sizeof(temp_buf), "%s", boot_args);
    int len = static_cast<int>(std::strlen(temp_buf));

    while (1) {
        mvprintw(field_y, field_x, "%-66s", "");
        mvprintw(field_y, field_x, "%s", temp_buf);
        move(field_y, field_x + len);
        refresh();

        int input_ch = getch();

        if (input_ch == KEY_ENTER || input_ch == 10 || input_ch == 13) {
            break;
        } else if (input_ch == KEY_BACKSPACE || input_ch == 127 || input_ch == '\b') {
            if (len > 0) temp_buf[--len] = '\0';
        } else if (input_ch >= 32 && input_ch <= 126) {
            if (len < max_len) {
                temp_buf[len++] = static_cast<char>(input_ch);
                temp_buf[len] = '\0';
            }
        }
    }

    std::snprintf(boot_args, sizeof(boot_args), "%s", temp_buf);
    curs_set(0);
    timeout(100);
}

void TuiSettingsPanel::handle_enter(int selected, int sy, int sx) {
    switch(selected) {
        case 0: palerain_flags ^= palerain_option_safemode; break;
        case 1: palerain_flags ^= palerain_option_verbose_boot; break;
        case 2: edit_boot_args(sy, sx); break;
        case 3: palerain_flags ^= palerain_option_force_revert; break;
        case 4: palerain_flags ^= palerain_option_flower_chain; break;
        case 5: GetFrame()->ShowMain(0); break;
    }
}

void TuiSettingsPanel::handle_device_update(const DeviceState& state) {
    return;
}

#endif // WITH_TUI

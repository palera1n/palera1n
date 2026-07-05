#ifdef WITH_TUI

#include "TuiSettingsPanel.hpp"

#include <ncurses.h>
#include <cstdio>
#include <cstring>

#include "Tui.hpp"
#include "TuiText.hpp"

#include "../globals.h"
#include "../paleinfo.h"

static const char *buttons[] = { "Back" };
static constexpr int kSettingsContentX = 2;
static constexpr int kSettingsContentWidth = 60;
static constexpr int kSettingsBootArgsWidth = 56;

TuiSettingsPanel::TuiSettingsPanel(TuiFrame* frame)
    : TuiPanel(frame) {}

const char** TuiSettingsPanel::get_buttons() const {
    return buttons;
}

void TuiSettingsPanel::draw(int sy, int sx, int selected) {
    const int content_x = sx + kSettingsContentX;
    const bool is_rootful = (palerain_flags & palerain_option_rootful) != 0;
    const int setup_selection = (palerain_flags & palerain_option_setup_rootful) ? 1
        : (palerain_flags & palerain_option_setup_partial_root) ? 2 : 0;

    tui_text::draw_wrapped_text(
        sy + 2,
        content_x,
        kSettingsContentWidth,
        "You may set the following options. If you don't know what they mean you'll probably have no reason to set them.",
        2
    );

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
        mvprintw(sy + 6 + i, content_x, "[%c] ", states[i] ? 'x' : ' ');
        if (selected == i) attron(A_REVERSE);
        printw("%s", labels[i]);
        if (selected == i) attroff(A_REVERSE);
    }

    mvprintw(sy + 8, sx + 6, "Boot Arguments:");
    if (selected == 2) attron(A_REVERSE);
    mvprintw(sy + 9, sx + 6, "%-56s", boot_args);
    if (selected == 2) attroff(A_REVERSE);

    for (int i = 2; i < 4; i++) {
        mvprintw(sy + 8 + i, content_x, "[%c] ", states[i] ? 'x' : ' ');
        if (selected == i + 1) attron(A_REVERSE);
        printw("%s", labels[i]);
        if (selected == i + 1) attroff(A_REVERSE);
    }

    mvprintw(sy + 13, content_x, "Jailbreak Type:");
    if (selected == 5) attron(A_REVERSE);
    mvprintw(sy + 14, content_x + 2, "(%c) Rootless", is_rootful ? ' ' : 'x');
    if (selected == 5) attroff(A_REVERSE);

    if (selected == 6) attron(A_REVERSE);
    mvprintw(sy + 15, content_x + 2, "(%c) Rootful", is_rootful ? 'x' : ' ');
    if (selected == 6) attroff(A_REVERSE);

    mvprintw(sy + 17, content_x, "Rootful Options:");

    for (int i = 0; i < 3; ++i) {
        const int item_idx = 7 + i;
        const int row = sy + 18 + i;
        const bool active = is_rootful;
        const bool is_selected = selected == item_idx;
        const bool checked = setup_selection == i;

        if (!active) attron(A_DIM);
        if (is_selected) attron(A_REVERSE);

        const char* option_label = i == 0 ? "Boot" : (i == 1 ? "Create FakeFS" : "Create BindFS");
        mvprintw(row, content_x + 2, "(%c) %s", checked ? 'x' : ' ', option_label);

        if (is_selected) attroff(A_REVERSE);
        if (!active) attroff(A_DIM);
    }
}

void TuiSettingsPanel::edit_boot_args(int start_y, int start_x) {
    curs_set(1);
    timeout(-1);

    int field_y = start_y + 9;
    int field_x = start_x + 6;
    int max_len = kSettingsBootArgsWidth;

    char temp_buf[128];
    std::snprintf(temp_buf, sizeof(temp_buf), "%s", boot_args);
    int len = static_cast<int>(std::strlen(temp_buf));

    while (1) {
        mvprintw(field_y, field_x, "%-56s", "");
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
        case 5:
            palerain_flags &= ~(palerain_option_rootless | palerain_option_rootful);
            palerain_flags &= ~(palerain_option_setup_rootful | palerain_option_setup_partial_root);
            palerain_flags |= palerain_option_rootless;
            break;
        case 6:
            palerain_flags &= ~(palerain_option_rootless | palerain_option_rootful);
            palerain_flags &= ~(palerain_option_setup_rootful | palerain_option_setup_partial_root);
            palerain_flags |= palerain_option_rootful;
            break;
        case 7:
            if (palerain_flags & palerain_option_rootful) {
                palerain_flags &= ~(palerain_option_setup_rootful | palerain_option_setup_partial_root);
            }
            break;
        case 8:
            if (palerain_flags & palerain_option_rootful) {
                palerain_flags &= ~(palerain_option_setup_rootful | palerain_option_setup_partial_root);
                palerain_flags |= palerain_option_setup_rootful;
            }
            break;
        case 9:
            if (palerain_flags & palerain_option_rootful) {
                palerain_flags &= ~(palerain_option_setup_rootful | palerain_option_setup_partial_root);
                palerain_flags |= palerain_option_setup_partial_root;
            }
            break;
        case 10: GetFrame()->ShowMain(0); break;
    }
}

void TuiSettingsPanel::handle_device_update(const DeviceState& state) {
    return;
}

bool TuiSettingsPanel::is_button_enabled(int btn_idx) const {
    if (btn_idx >= 7 && btn_idx <= 9) {
        return (palerain_flags & palerain_option_rootful) != 0;
    }

    return true;
}

#endif // WITH_TUI

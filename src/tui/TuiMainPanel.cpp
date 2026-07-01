#ifdef WITH_TUI

#include "TuiMainPanel.hpp"
#include "Tui.hpp"
#include <ncurses.h>
#include <cstring>
#include <mutex>

#include "../state.hpp"

static const char *buttons[] = { "[ Options ]", "[ Start ]", "[ Quit ]" };

TuiMainPanel::TuiMainPanel(TuiFrame* frame)
    : TuiPanel(frame) {}

const char** TuiMainPanel::get_buttons() const {
    return buttons;
}

void TuiMainPanel::draw(int sy, int sx, int selected) {
    DeviceState state = GetFrame()->GetDeviceState();

    if (state.connected) {
        mvprintw(sy + 4, sx + 4, "connected");
        mvprintw(sy + 5, sx + 4, "%s", state.productType.c_str());
        mvprintw(sy + 6, sx + 4, "%s", state.productVersion.c_str());
        mvprintw(sy + 7, sx + 4, "ECID: 0x%llX", (unsigned long long)state.ecid);

        if (!state.isSupported) {
            attron(A_BOLD);
            mvprintw(sy + 9, sx + 4, "unsupported");
            attroff(A_BOLD);
        }
    } else {
        mvprintw(sy + 4, sx + 4, "waiting");
    }
}

void TuiMainPanel::handle_enter(int selected, int sy, int sx) {
    switch(selected) {
        case 0:
            GetFrame()->ShowSettings(5);
            break;

        case 1: {
            DeviceState state = GetFrame()->GetDeviceState();
            bool can_start = (state.connected && state.isSupported && state.mode != DeviceMode::DFU);

            if (can_start) {
                if (state.mode == DeviceMode::Recovery) {
                    GetFrame()->ShowDfu(1);
                } else {
                    GetFrame()->ShowRecovery(1);
                }
            }
            break;
        }

        case 2:
            GetFrame()->Stop();
            break;
    }
}

void TuiMainPanel::handle_device_update(const DeviceState& state) {}

bool TuiMainPanel::is_button_enabled(int btn_idx) const {
    DeviceState state = GetFrame()->GetDeviceState();

    switch(btn_idx) {
        case 0: return true;
        case 1: return (state.connected && state.isSupported && state.mode != DeviceMode::DFU);
        case 2: return true;
    }

    return false;
}

#endif // WITH_TUI

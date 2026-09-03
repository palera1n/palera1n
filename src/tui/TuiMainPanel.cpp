/*
 * palera1n - https://palera.in
 *
 * Copyright (C) 2026 palera1n team
 *
 * SPDX-License-Identifier: MIT
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 */

#ifdef WITH_TUI

#include "TuiMainPanel.hpp"

#include <string>
#include <ncurses.h>

#include "Tui.hpp"
#include "TuiText.hpp"
#include "../events/event.hpp"
#include "../globals.h"
#include "../paleinfo.h"

static const char *buttons[] = { "Options", "Start", "Quit" };
static constexpr int kMainContentX = 2;
static constexpr int kMainContentWidth = 60;
static constexpr int kMainSeparatorX = 1;
static constexpr int kMainSeparatorWidth = 63;

static std::string get_device_title(const DeviceState& state) {
    if (state.multipleDevices || state.connectedDeviceCount > 1) {
        return "Multiple devices detected";
    }
    if (!state.connected) {
        return "No device connected";
    }

    const std::string product = state.displayName.empty() ? (state.productType.empty() ? "Unknown" : state.productType) : state.displayName;
    return product;
}

static std::string get_device_subtitle(const DeviceState& state) {
    if (state.multipleDevices || state.connectedDeviceCount > 1) {
        return std::to_string(state.connectedDeviceCount) +
            " USB devices connected. Disconnect extras and keep only one device attached.";
    }
    if (!state.connected) {
        return "Please connect a device to get started. Ensure version range is 15.0+";
    }

    const std::string version = state.productVersion.empty() ? "Unknown" : state.productVersion;
    const std::string ecid = state.ecid != 0 ? ("ECID: " + std::to_string(state.ecid)) : "ECID: Unknown";

    switch (state.mode) {
        case DeviceMode::Normal:
            if (state.requiresCLI) {
                return "Sorry, jailbreaking is only available via DFU mode * iOS " + version + "\n" + ecid;
            } else if (state.isSupported) {
                return "Connected in normal mode * iOS " + version + "\n" + ecid;
            } else {
                return "Sorry, this normal device is not supported * iOS " + version + "\n" + ecid;
            }
        case DeviceMode::Recovery:
            if (state.requiresCLI) {
                return "Sorry, jailbreaking is only available via DFU mode.\n" + ecid;
            } else if (state.isSupported) {
                return "Connected in recovery mode.\n" + ecid;
            } else {
                return "Sorry, this recovery device is not supported.\n" + ecid;
            }
        case DeviceMode::DFU:
           if (state.isSupported) {
                return "Connected in DFU mode.\n" + ecid;
            } else {
                return "Sorry, this DFU device is not supported.\n" + ecid;
            }
        case DeviceMode::Pongo:
            if (state.isSupported) {
                return "Connected in PongoOS mode.\n" + ecid;
            } else {
                return "This PongoOS device is not supported, worth a shot though...\n" + ecid;
            }
        case DeviceMode::None:
            return "Please connect a device to get started. Ensure version range is 15.0+";
    }

    return "";
}

TuiMainPanel::TuiMainPanel(TuiFrame* frame)
    : TuiPanel(frame) {}

const char** TuiMainPanel::get_buttons() const {
    return buttons;
}

void TuiMainPanel::draw(int sy, int sx, int selected) {
    const DeviceState state = GetFrame()->GetDeviceState();
    const int content_x = sx + kMainContentX;
    int y = sy + 2;
    const int bottom_y = sy + 22;
    const int content_bottom = bottom_y - 1;

    (void)selected;

    const std::string title = get_device_title(state);
    const std::string subtitle = get_device_subtitle(state);

    attron(A_BOLD);
    mvprintw(y, content_x, "%.*s", kMainContentWidth, title.c_str());
    attroff(A_BOLD);
    ++y;

    if (has_colors()) {
        attron(COLOR_PAIR(1));
    }
    y += tui_text::draw_wrapped_text(y, content_x, kMainContentWidth, subtitle, 2);
    if (has_colors()) {
        attroff(COLOR_PAIR(1));
    }
    ++y;

    if (y <= content_bottom) {
        mvhline(y, sx + kMainSeparatorX, ACS_HLINE, kMainSeparatorWidth);
        y += 2;
    }

    const std::string made_by =
        "Made by: asdfugil, claration, kok3shidoll, mineek, sarah, staturnz";
    const std::string thanks_to =
        "Thanks to: itsnebulalol, llsc12, lrdsnow, dedbeddedbed, kirb, ehilwyma, "
        "opa334, 0x7ff, alfiecg24, sneko, sbingner, nikias, "
        "tihmstar, Checkra1n (Siguza, axi0mx, littlelailo et al.), Procursus "
        "(Hayden Seay, Cameron Katri, Keto et al.)";

    if (y <= content_bottom) {
        y += tui_text::draw_wrapped_text(y, content_x, kMainContentWidth, made_by, 2);
    }

    if (y <= content_bottom) {
        ++y;
    }

    if (y <= content_bottom) {
        y += tui_text::draw_wrapped_text(y, content_x, kMainContentWidth, thanks_to, 4);
    }

    if (y <= content_bottom) {
        ++y;
    }

    if (y <= content_bottom) {
        mvhline(y, sx + kMainSeparatorX, ACS_HLINE, kMainSeparatorWidth);
        y += 1;
    }

    if (y <= content_bottom) {
        mvprintw(y, content_x, "Twitter: @palera1n");
        ++y;
    }
    if (y <= content_bottom) {
        mvprintw(y, content_x, "Website: https://palera.in");
        ++y;
    }

    if (y <= content_bottom) {
        attron(A_BOLD);
        y += tui_text::draw_wrapped_text(
            y,
            content_x,
            kMainContentWidth,
            "NOTE: Please ensure you've made a backup of your device before proceeding.",
            2
        );
        attroff(A_BOLD);
    }

    if (y <= content_bottom) {
        mvhline(y, sx + kMainSeparatorX, ACS_HLINE, kMainSeparatorWidth);
        y += 2;
    }

    const bool quick_mode_enabled = (palerain_flags & palerain_option_quick) != 0;
    const std::string quick_mode_prefix = std::string("[") + (quick_mode_enabled ? "x" : " ") + "] ";
    const std::string quick_mode_label = "Enable Quick Mode";
    const int quick_mode_total_width = static_cast<int>(quick_mode_prefix.size() + quick_mode_label.size());
    const int quick_mode_x = content_x + std::max(0, kMainContentWidth - quick_mode_total_width);

    attron(A_DIM);
    mvprintw(bottom_y, content_x, "%.*s", kMainContentWidth, "Made with <3 by C (claration)");
    attroff(A_DIM);

    mvprintw(bottom_y, quick_mode_x, "%s", quick_mode_prefix.c_str());

    if (selected == 0) {
        attron(A_REVERSE);
    }
    mvprintw(bottom_y, quick_mode_x + static_cast<int>(quick_mode_prefix.size()), "%s", quick_mode_label.c_str());
    if (selected == 0) {
        attroff(A_REVERSE);
    }
}

void TuiMainPanel::handle_enter(int selected, int sy, int sx) {
    DeviceState state = GetFrame()->GetDeviceState();
    const bool block_start = state.multipleDevices || state.connectedDeviceCount > 1;

    switch(selected) {
        case 0:
            palerain_flags ^= palerain_option_quick;
            break;

        case 1:
            GetFrame()->ShowSettings(5);
            break;
        case 2: {
            if (block_start) break;

            bool isDfuLike =
                state.mode == DeviceMode::DFU ||
                state.mode == DeviceMode::Pongo;

            bool can_start =
                state.connected &&
                state.isSupported &&
                (isDfuLike || !state.requiresCLI);

            if (can_start) {
                if (state.mode == DeviceMode::Recovery) {
                    GetFrame()->ShowDfu(1);
                } else if (isDfuLike) {
                    GetFrame()->ShowExploit(0);
                } else {
                    GetFrame()->ShowRecovery(1);
                }
            }
            break;
        }

        case 3:
            GetFrame()->Stop();
            break;
    }
}

void TuiMainPanel::handle_device_update(const DeviceState& state) {}

bool TuiMainPanel::is_button_enabled(int btn_idx) const {
    DeviceState state = GetFrame()->GetDeviceState();

    if (state.multipleDevices || state.connectedDeviceCount > 1) {
        switch(btn_idx) {
            case 0: return true;
            case 1: return true;
            case 2: return false;
            case 3: return true;
        }
    }

    switch(btn_idx) {
        case 0: return true;
        case 1: return true;
        case 2: {
            bool isDfuLike =
                state.mode == DeviceMode::DFU ||
                state.mode == DeviceMode::Pongo;

            return state.connected && state.isSupported && (isDfuLike || !state.requiresCLI);
        }
        case 3: return true;
    }

    return false;
}

#endif // WITH_TUI

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

#ifndef TUI_PANEL_HPP
#define TUI_PANEL_HPP

#include <ncurses.h>

#include "../events/event.hpp"

class TuiFrame;

enum PanelState {
    PANEL_MAIN = 0,
    PANEL_OPTIONS,
    PANEL_START_1,
    PANEL_START_2,
    PANEL_FINAL,
    PANEL_COUNT
};

class TuiPanel {
public:
    explicit TuiPanel(TuiFrame* frame)
        : m_frame(frame) {}

    virtual ~TuiPanel() = default;

    virtual void on_show() {}
    virtual void update() {}

    virtual void draw(int sy, int sx, int selected) = 0;
    virtual void handle_enter(int selected, int sy, int sx) = 0;
    virtual void handle_device_update(const DeviceState& state) = 0;
    virtual bool is_button_enabled(int btn_idx) const { return true; }

    virtual int get_total_items() const = 0;
    virtual int get_button_count() const = 0;
    virtual const char** get_buttons() const = 0;
protected:
    TuiFrame* GetFrame() const {
        return m_frame;
    }
private:
    TuiFrame* m_frame = nullptr;
};

#endif // TUI_PANEL_HPP

#endif // WITH_TUI

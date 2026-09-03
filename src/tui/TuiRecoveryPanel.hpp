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

#ifndef TUI_RECOVERY_PANEL_HPP
#define TUI_RECOVERY_PANEL_HPP

#include "TuiPanel.hpp"

#include <string>
#include <atomic>

class TuiRecoveryPanel : public TuiPanel {
public:
    explicit TuiRecoveryPanel(TuiFrame* frame);
    ~TuiRecoveryPanel() override = default;

    void SetStatusText(const std::string& text);

    void on_show() override;
    void draw(int sy, int sx, int selected) override;
    void handle_enter(int selected, int sy, int sx) override;
    void handle_device_update(const DeviceState& state) override;
    bool is_button_enabled(int btn_idx) const override;

    int get_total_items() const override { return 2; }
    int get_button_count() const override { return 2; }
    const char** get_buttons() const override;

private:
    void enter_recovery_mode();

    std::string m_status_text;
    bool m_preserve_status_on_show = false;
    std::atomic<bool> m_is_entering_recovery;
    bool m_buttons_disabled;
};

#endif // TUI_RECOVERY_PANEL_HPP

#endif // WITH_TUI

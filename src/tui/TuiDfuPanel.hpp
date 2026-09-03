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

#ifndef TUI_DFU_PANEL_HPP
#define TUI_DFU_PANEL_HPP

#include "TuiPanel.hpp"
#include "../sequence.hpp"
#include <string>
#include <chrono>

class TuiDfuPanel : public TuiPanel {
public:
    explicit TuiDfuPanel(TuiFrame* frame);
    ~TuiDfuPanel() override = default;

    void on_show() override;
    void update() override;
    void draw(int sy, int sx, int selected) override;
    void handle_enter(int selected, int sy, int sx) override;
    void handle_device_update(const DeviceState& state) override;
    bool is_button_enabled(int btn_idx) const override;

    int get_total_items() const override { return 2; }
    int get_button_count() const override { return 2; }
    const char** get_buttons() const override;

private:
    void load_sequence_for_product(const std::string& product_type, bool reset_state);
    void reset_sequence_state();
    void start_sequence();
    void reboot();
    void update_sequence_timer();
    int scale_coord(int value, int input_span, int output_span) const;
    void draw_sequence_buttons(int button_x, int button_y, int button_width, int button_height) const;
    void draw_wrapped_steps(int steps_x, int steps_width, int max_lines, int bottom_y) const;

    DfuSequence m_sequence;
    std::string m_current_product_type;
    size_t m_index = 0;
    int m_stepRemaining = -1;
    int m_actionExecutedIndex = -1;
    bool m_isEnteringDfu = false;
    bool m_dfuSuccess = false;
    bool m_waitingForDfuTransition = false;
    std::chrono::steady_clock::time_point m_dfuSuccessAt;
    std::chrono::steady_clock::time_point m_last_tick;
};

#endif // TUI_DFU_PANEL_HPP

#endif // WITH_TUI

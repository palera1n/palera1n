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

#include "Tui.hpp"

#include <algorithm>
#include <cstdio>
#include <cstring>
#include <clocale>
#include <string>
#include <mutex>
#include <vector>
#include <memory>

#include <ncurses.h>

#include "TuiPanel.hpp"
#include "TuiMainPanel.hpp"
#include "TuiSettingsPanel.hpp"
#include "TuiRecoveryPanel.hpp"
#include "TuiDfuPanel.hpp"
#include "TuiExploitPanel.hpp"
#include "TuiBranding.hpp"
#include "../globals.h"
#include "../paleinfo.h"
#include "../events/event.hpp"

#define WIDTH 80
#define HEIGHT 24

static constexpr int LEFT_PANEL_WIDTH = 65;
static constexpr int RIGHT_PANEL_WIDTH = WIDTH - LEFT_PANEL_WIDTH;
static constexpr int RIGHT_INNER_WIDTH = RIGHT_PANEL_WIDTH - 2;
static constexpr int MAIN_BUTTON_VERTICAL_GAP = 1;
static constexpr int SIDEBAR_BUTTON_WIDTH = 13;
static constexpr int SIDEBAR_WALL_GAP = 1;
static constexpr short COLOR_PAIR_SUBTITLE = 1;

static std::string trim_button_label(std::string label) {
    while (!label.empty() && label.front() == ' ') {
        label.erase(label.begin());
    }
    while (!label.empty() && label.back() == ' ') {
        label.pop_back();
    }

    return label;
}

static std::string format_sidebar_button_label(const char* raw_label, int button_width) {
    if (button_width < 2) {
        return "";
    }

    if (!raw_label) {
        return std::string(static_cast<size_t>(button_width), ' ');
    }

    std::string label(raw_label);
    if (label.size() >= 2 && label.front() == '[' && label.back() == ']') {
        label = label.substr(1, label.size() - 2);
    }

    label = trim_button_label(label);

    const int inner_width = button_width - 2;
    if (inner_width <= 0) {
        return "[]";
    }

    if (static_cast<int>(label.size()) > inner_width) {
        label.resize(static_cast<size_t>(inner_width));
    }

    const int left_pad = std::max(0, (inner_width - static_cast<int>(label.size())) / 2);
    const int right_pad = std::max(0, inner_width - static_cast<int>(label.size()) - left_pad);

    return "[" + std::string(static_cast<size_t>(left_pad), ' ') + label + std::string(static_cast<size_t>(right_pad), ' ') + "]";
}

static void draw_panel_border(int y, int x, int width, int height, const char* title = nullptr) {
    if (title) {
        char title_buf[64];
        std::snprintf(title_buf, sizeof(title_buf), "[ %s ]", title);
        int title_len = static_cast<int>(std::strlen(title_buf));
        int title_start_x = x + (width - title_len) / 2;

        mvhline(y, x, ACS_HLINE, title_start_x - x);
        mvprintw(y, title_start_x, "%s", title_buf);
        mvhline(y, title_start_x + title_len, ACS_HLINE, (x + width) - (title_start_x + title_len));
    } else {
        mvhline(y, x, ACS_HLINE, width);
    }

    mvhline(y + height - 1, x, ACS_HLINE, width);
    mvvline(y, x, ACS_VLINE, height);
    mvvline(y, x + width - 1, ACS_VLINE, height);

    mvaddch(y, x, ACS_ULCORNER);
    mvaddch(y, x + width - 1, ACS_URCORNER);
    mvaddch(y + height - 1, x, ACS_LLCORNER);
    mvaddch(y + height - 1, x + width - 1, ACS_LRCORNER);
}

static TuiFrame* g_frame = nullptr;

TuiFrame::TuiFrame() {
    InitPanels();
    g_frame = this;
    InitDeviceEventListeners();
}

TuiFrame::~TuiFrame() {
    if (g_frame == this) {
        g_frame = nullptr;
    }
}

void TuiFrame::InitPanels() {
    m_panels.resize(PANEL_COUNT);
    m_panels[PANEL_MAIN] = std::make_unique<TuiMainPanel>(this);
    m_panels[PANEL_OPTIONS] = std::make_unique<TuiSettingsPanel>(this);
    m_panels[PANEL_START_1] = std::make_unique<TuiRecoveryPanel>(this);
    m_panels[PANEL_START_2] = std::make_unique<TuiDfuPanel>(this);
    m_panels[PANEL_FINAL] = std::make_unique<TuiExploitPanel>(this);
}

void TuiFrame::InitDeviceEventListeners() {
    register_device_state_callback([](const DeviceState& state) {
        update_tui_device_state(&state);
    });
    ensure_device_event_system_started();
}

void TuiFrame::ActivatePanelIfNeeded(PanelState previous_panel) {
    if (m_currentPanel == previous_panel) {
        return;
    }

    if (m_panels[m_currentPanel]) {
        m_panels[m_currentPanel]->on_show();
    }
}

void TuiFrame::DrawUi() {
    int rows, cols;
    getmaxyx(stdscr, rows, cols);

    erase();

    if (rows < HEIGHT || cols < WIDTH) {
        mvprintw(rows / 2, (cols - 34) / 2, "Terminal too small, need 80x24");
        refresh();
        return;
    }

    int start_y = (rows - HEIGHT) / 2;
    int start_x = (cols - WIDTH) / 2;


    const int left_x = start_x;
    const int right_x = start_x + LEFT_PANEL_WIDTH;

    draw_panel_border(start_y, left_x, LEFT_PANEL_WIDTH, HEIGHT, "palera1n - Version beta " PALERAIN_VERSION);
    draw_panel_border(start_y, right_x, RIGHT_PANEL_WIDTH, HEIGHT);

    if (m_panels[m_currentPanel]) {
        m_panels[m_currentPanel]->draw(start_y, left_x, m_selected);
    }

    if (m_panels[m_currentPanel]) {
        int btn_cnt = m_panels[m_currentPanel]->get_button_count();
        int total_items = m_panels[m_currentPanel]->get_total_items();
        const char **panel_buttons = m_panels[m_currentPanel]->get_buttons();

        const int sidebar_x = right_x;
        const int sidebar_inner_x = sidebar_x + 1;
        const int sidebar_inner_width = RIGHT_INNER_WIDTH;
        const int sidebar_top = start_y + 1;
        const int sidebar_bottom = start_y + HEIGHT - 2;
        const int content_top = sidebar_top + SIDEBAR_WALL_GAP;
        const int content_bottom = sidebar_bottom;
        const int content_x = sidebar_inner_x + SIDEBAR_WALL_GAP;
        const int content_width = std::max(1, sidebar_inner_width - (SIDEBAR_WALL_GAP * 2));

        draw_tui_sidebar_branding(sidebar_top, sidebar_inner_x, sidebar_inner_width);

        std::vector<std::string> labels;
        labels.reserve(static_cast<size_t>(btn_cnt));
        const int button_width = std::clamp(SIDEBAR_BUTTON_WIDTH, 8, content_width);

        for (int i = 0; i < btn_cnt; ++i) {
            labels.push_back(format_sidebar_button_label(panel_buttons[i], button_width));
        }

        const int buttons_block_height = std::max(1, btn_cnt + ((btn_cnt - 1) * MAIN_BUTTON_VERTICAL_GAP));
        const int first_y = std::max(content_top, content_bottom - buttons_block_height + 1);
        const int button_x = content_x + std::max(0, (content_width - button_width) / 2);

        for (int i = 0; i < btn_cnt; ++i) {
            if (i > 0) {
                const int separator_y = first_y + (i * (MAIN_BUTTON_VERTICAL_GAP + 1)) - 1;
                mvhline(separator_y, sidebar_inner_x, ACS_HLINE, sidebar_inner_width);
            }

            int btn_idx = total_items - btn_cnt + i;
            bool enabled = m_panels[m_currentPanel]->is_button_enabled(btn_idx);
            const std::string& label = labels[static_cast<size_t>(i)];

            if (!enabled) {
                attron(A_DIM);
                mvprintw(first_y + (i * (MAIN_BUTTON_VERTICAL_GAP + 1)), button_x, "%s", label.c_str());
                attroff(A_DIM);
            } else {
                if (btn_idx == m_selected) attron(A_REVERSE);
                mvprintw(first_y + (i * (MAIN_BUTTON_VERTICAL_GAP + 1)), button_x, "%s", label.c_str());
                if (btn_idx == m_selected) attroff(A_REVERSE);
            }
        }
    }

    refresh();
}

void TuiFrame::HandleInput(int ch, int start_y, int start_x) {
    if (ch == ERR || !m_panels[m_currentPanel]) return;

    int total_items = m_panels[m_currentPanel]->get_total_items();
    if (total_items == 0) return;

    PanelState previous_panel = m_currentPanel;

    switch(ch) {
        case KEY_UP:
        case KEY_LEFT: {
            int prev = m_selected;
            do {
                prev = (prev - 1 + total_items) % total_items;
            } while (!m_panels[m_currentPanel]->is_button_enabled(prev) && prev != m_selected);
            m_selected = prev;
            break;
        }

        case KEY_DOWN:
        case KEY_RIGHT: {
            int next = m_selected;
            do {
                next = (next + 1) % total_items;
            } while (!m_panels[m_currentPanel]->is_button_enabled(next) && next != m_selected);
            m_selected = next;
            break;
        }

        case KEY_ENTER:
        case 10:
        case 13:
            if (!m_panels[m_currentPanel]->is_button_enabled(m_selected)) return;
            m_panels[m_currentPanel]->handle_enter(m_selected, start_y, start_x);
            break;
    }

    ActivatePanelIfNeeded(previous_panel);
}

void TuiFrame::Run() {
    m_running = true;
    std::setlocale(LC_ALL, "");
    initscr();
    noecho();
    cbreak();
    curs_set(0);
    keypad(stdscr, TRUE);
    timeout(100);

    if (has_colors()) {
        start_color();
        use_default_colors();
        init_pair(COLOR_PAIR_SUBTITLE, COLOR_YELLOW, -1);
    }

    int ch;

    while (m_running) {
        PanelState previous_panel = m_currentPanel;

        if (m_panels[m_currentPanel]) {
            m_panels[m_currentPanel]->update();
        }

        ActivatePanelIfNeeded(previous_panel);

        DrawUi();
        ch = getch();

        if (ch == 'q' || ch == 'Q') {
            Stop();
            continue;
        }

        int rows, cols;
        getmaxyx(stdscr, rows, cols);
        if (rows >= HEIGHT && cols >= WIDTH) {
            int start_y = (rows - HEIGHT) / 2;
            int start_x = (cols - WIDTH) / 2;
            HandleInput(ch, start_y, start_x);
        }
    }

    endwin();
}

void TuiFrame::ShowMain(int selected) {
    m_currentPanel = PANEL_MAIN;
    m_selected = selected;
}

void TuiFrame::ShowSettings(int selected) {
    m_currentPanel = PANEL_OPTIONS;
    m_selected = selected;
}

void TuiFrame::ShowRecovery(int selected) {
    m_currentPanel = PANEL_START_1;
    m_selected = selected;
}

void TuiFrame::ShowDfu(int selected) {
    m_currentPanel = PANEL_START_2;
    m_selected = selected;
}

void TuiFrame::ShowExploit(int selected) {
    m_currentPanel = PANEL_FINAL;
    m_selected = selected;
}

PanelState TuiFrame::GetCurrentPanel() const {
    return m_currentPanel;
}

int TuiFrame::GetSelected() const {
    return m_selected;
}

void TuiFrame::SetSelected(int selected) {
    m_selected = selected;
}

bool TuiFrame::IsRunning() const {
    return m_running;
}

void TuiFrame::Stop() {
    m_running = false;
}

DeviceState TuiFrame::GetDeviceState() const {
    std::lock_guard<std::mutex> lock(m_deviceMutex);
    return m_device;
}

void TuiFrame::UpdateDeviceState(const DeviceState& state) {
    {
        std::lock_guard<std::mutex> lock(m_deviceMutex);
        m_device = state;
    }

    const PanelState previous_panel = m_currentPanel;
    if (m_currentPanel != PANEL_MAIN && m_currentPanel != PANEL_OPTIONS && m_panels[m_currentPanel]) {
        m_panels[m_currentPanel]->handle_device_update(state);
        ActivatePanelIfNeeded(previous_panel);
    }
}

TuiMainPanel* TuiFrame::GetMainPanel() const {
    return static_cast<TuiMainPanel*>(m_panels[PANEL_MAIN].get());
}

TuiSettingsPanel* TuiFrame::GetSettingsPanel() const {
    return static_cast<TuiSettingsPanel*>(m_panels[PANEL_OPTIONS].get());
}

TuiRecoveryPanel* TuiFrame::GetRecoveryPanel() const {
    return static_cast<TuiRecoveryPanel*>(m_panels[PANEL_START_1].get());
}

TuiDfuPanel* TuiFrame::GetDfuPanel() const {
    return static_cast<TuiDfuPanel*>(m_panels[PANEL_START_2].get());
}

TuiExploitPanel* TuiFrame::GetExploitPanel() const {
    return static_cast<TuiExploitPanel*>(m_panels[PANEL_FINAL].get());
}

void update_tui_device_state(const DeviceState* new_state) {
    if (!g_frame || !new_state) {
        return;
    }

    g_frame->UpdateDeviceState(*new_state);
}

void ui_run(void) {
    TuiFrame frame;
    frame.Run();
}

#endif // WITH_TUI

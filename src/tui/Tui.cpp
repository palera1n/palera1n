#ifdef WITH_TUI

#include "Tui.hpp"

#include <ncurses.h>
#include <cstdio>
#include <cstring>
#include <mutex>
#include <thread>
#include <chrono>
#include <vector>
#include <memory>

#include "TuiPanel.hpp"
#include "TuiMainPanel.hpp"
#include "TuiSettingsPanel.hpp"
#include "TuiRecoveryPanel.hpp"
#include "TuiDfuPanel.hpp"
#include "TuiExploitPanel.hpp"

#include "../globals.h"
#include "../utils.h"
#include "../paleinfo.h"
#include "../state.hpp"
#include "../event.hpp"

#define WIDTH 80
#define HEIGHT 24

static TuiFrame* g_frame = nullptr;

TuiFrame::TuiFrame() {
    InitPanels();
    InitDeviceEventListeners();
}

TuiFrame::~TuiFrame() = default;

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

    std::thread([this]() {
        idevice_event_subscribe(normal_device_event_cb, nullptr);
        while (m_running) {
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        }
    }).detach();

    std::thread([this]() {
        static irecv_device_event_context_t ctx = nullptr;
        irecv_device_event_subscribe(&ctx, recovery_device_event_cb, nullptr);
        while (m_running) {
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        }
    }).detach();
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
        mvprintw(rows / 2, (cols - 34) / 2, "Terminal too small! Need min 80x24.");
        refresh();
        return;
    }

    int start_y = (rows - HEIGHT) / 2;
    int start_x = (cols - WIDTH) / 2;

    const char *title = "palera1n - Version beta " PALERAIN_VERSION;
    char title_buf[64];
    std::snprintf(title_buf, sizeof(title_buf), "[ %s ]", title);
    int title_len = static_cast<int>(std::strlen(title_buf));
    int title_start_x = start_x + (WIDTH - title_len) / 2;

    mvhline(start_y, start_x, ACS_HLINE, title_start_x - start_x);
    mvprintw(start_y, title_start_x, "%s", title_buf);
    mvhline(start_y, title_start_x + title_len, ACS_HLINE, (start_x + WIDTH) - (title_start_x + title_len));

    mvhline(start_y + HEIGHT - 1, start_x, ACS_HLINE, WIDTH);
    mvvline(start_y, start_x, ACS_VLINE, HEIGHT);
    mvvline(start_y, start_x + WIDTH - 1, ACS_VLINE, HEIGHT);

    mvaddch(start_y, start_x, ACS_ULCORNER);
    mvaddch(start_y, start_x + WIDTH - 1, ACS_URCORNER);
    mvaddch(start_y + HEIGHT - 1, start_x, ACS_LLCORNER);
    mvaddch(start_y + HEIGHT - 1, start_x + WIDTH - 1, ACS_LRCORNER);

    if (m_panels[m_currentPanel]) {
        m_panels[m_currentPanel]->draw(start_y, start_x, m_selected);
    }

    if (m_panels[m_currentPanel]) {
        int btn_cnt = m_panels[m_currentPanel]->get_button_count();
        int total_items = m_panels[m_currentPanel]->get_total_items();
        const char **panel_buttons = m_panels[m_currentPanel]->get_buttons();

        int y = start_y + 22;
        int x = start_x + WIDTH - 2;

        for (int i = btn_cnt - 1; i >= 0; i--) {
            int len = static_cast<int>(std::strlen(panel_buttons[i]));
            x -= len;

            int btn_idx = total_items - btn_cnt + i;
            bool enabled = m_panels[m_currentPanel]->is_button_enabled(btn_idx);

            if (!enabled) {
                attron(A_DIM);
                mvprintw(y, x, "%s", panel_buttons[i]);
                attroff(A_DIM);
            } else {
                if (btn_idx == m_selected) attron(A_REVERSE);
                mvprintw(y, x, "%s", panel_buttons[i]);
                if (btn_idx == m_selected) attroff(A_REVERSE);
            }

            x -= 3;
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

    initscr();
    noecho();
    cbreak();
    curs_set(0);
    keypad(stdscr, TRUE);
    timeout(100);

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
    g_frame = &frame;
    frame.Run();
    g_frame = nullptr;
}

#endif // WITH_TUI

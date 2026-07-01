#ifdef WITH_TUI

#include "TuiDfuPanel.hpp"
#include "Tui.hpp"
#include "TuiRecoveryPanel.hpp"

#include <algorithm>
#include <thread>
#include <chrono>
#include <ncurses.h>
#include <libirecovery.h>

#include "../sequence.hpp"
#include "../state.hpp"
#include "../globals.h"
#include "../paleinfo.h"

static const char *buttons[] = { "[ Back ]", "[ Start ]" };
static constexpr int kDfuPanelCoordinateWidth = 310;
static constexpr int kDfuPanelCoordinateHeight = 260;
static constexpr int kTuiDeviceCanvasWidth = 52;

TuiDfuPanel::TuiDfuPanel(TuiFrame* frame)
    : TuiPanel(frame) {}

const char** TuiDfuPanel::get_buttons() const {
    return buttons;
}

void TuiDfuPanel::reset_sequence_state() {
    m_index = 0;
    m_stepRemaining = -1;
    m_actionExecutedIndex = -1;
    m_isEnteringDfu = false;
    m_dfuSuccess = false;
    m_waitingForDfuTransition = false;
    m_dfuSuccessAt = std::chrono::steady_clock::time_point{};
    m_last_tick = std::chrono::steady_clock::time_point{};
}

void TuiDfuPanel::load_sequence_for_product(const std::string& product_type, bool reset_state) {
    if (m_current_product_type != product_type) {
        m_current_product_type = product_type;
        m_sequence = ParseSequence(product_type);
    }

    if (reset_state) {
        reset_sequence_state();
    }
}

void TuiDfuPanel::on_show() {
    const DeviceState state = GetFrame()->GetDeviceState();
    load_sequence_for_product(state.productType, true);
    GetFrame()->SetSelected(1);

    if (palerain_flags & palerain_option_quick) {
        start_sequence();
    }
}

void TuiDfuPanel::update() {
    if (!m_waitingForDfuTransition || !m_dfuSuccess) {
        return;
    }

    const auto now = std::chrono::steady_clock::now();
    if (std::chrono::duration_cast<std::chrono::seconds>(now - m_dfuSuccessAt).count() < 3) {
        return;
    }

    m_waitingForDfuTransition = false;
    GetFrame()->ShowExploit(0);
}

bool TuiDfuPanel::is_button_enabled(int btn_idx) const {
    (void)btn_idx;
    return !m_isEnteringDfu && !m_dfuSuccess;
}

void TuiDfuPanel::start_sequence() {
    if (m_sequence.steps.empty()) {
        return;
    }

    m_isEnteringDfu = true;
    m_dfuSuccess = false;
    m_index = 0;
    m_stepRemaining = -1;
    m_actionExecutedIndex = -1;
    m_last_tick = std::chrono::steady_clock::now();
}

void TuiDfuPanel::update_sequence_timer() {
    if (!m_isEnteringDfu || m_sequence.steps.empty() || m_index >= m_sequence.steps.size()) {
        return;
    }

    auto now = std::chrono::steady_clock::now();
    if (std::chrono::duration_cast<std::chrono::seconds>(now - m_last_tick).count() >= 1) {
        m_last_tick = now;

        const auto& step = m_sequence.steps[m_index];
        if (m_stepRemaining < 0) {
            m_stepRemaining = step.duration;
        }

        if (m_stepRemaining > 0) {
            --m_stepRemaining;
        }

        if (m_stepRemaining == 0) {
            ++m_index;
            m_stepRemaining = -1;
        }

        if (m_index < m_sequence.steps.size()) {
            const auto& activeStep = m_sequence.steps[m_index];
            if (!activeStep.action.empty() && m_actionExecutedIndex != (int)m_index) {
                m_actionExecutedIndex = m_index;
                if (activeStep.action == "reboot") {
                    reboot();
                }
            }
        }
    }
}

int TuiDfuPanel::scale_coord(int value, int input_span, int output_span) const {
    if (input_span <= 0 || output_span <= 1) {
        return 0;
    }

    return (value * (output_span - 1)) / input_span;
}

void TuiDfuPanel::draw_sequence_buttons(int sy, int sx, int button_x, int button_y, int button_width, int button_height) const {
    if (m_sequence.buttons.empty()) {
        return;
    }

    std::vector<std::string> active_buttons;
    if (m_isEnteringDfu && m_index < m_sequence.steps.size()) {
        active_buttons = m_sequence.steps[m_index].activeButtons;
    }

    for (const auto& btn : m_sequence.buttons) {
        const int mapped_x = button_x + scale_coord(btn.x, kDfuPanelCoordinateWidth, std::max(1, button_width - 1));
        const int mapped_y = button_y + scale_coord(btn.y, kDfuPanelCoordinateHeight, std::max(1, button_height - 1));

        std::string label = btn.name;
        const int label_x = std::max(button_x, mapped_x);

        const bool is_active = std::find(active_buttons.begin(), active_buttons.end(), btn.id) != active_buttons.end();
        if (is_active) {
            attron(A_BOLD);
        } else {
            attron(A_DIM);
        }

        mvprintw(mapped_y, label_x, "%s", label.c_str());

        if (is_active) {
            attroff(A_BOLD);
        } else {
            attroff(A_DIM);
        }
    }
}

void TuiDfuPanel::draw_wrapped_steps(int sy, int sx, int steps_x, int steps_width) const {
    const int right_edge = steps_x + steps_width;
    struct StepBlock {
        std::vector<std::string> lines;
        size_t width = 0;
        size_t prefix_size = 0;
    };

    std::vector<StepBlock> blocks;
    blocks.reserve(m_sequence.steps.size());

    for (size_t i = 0; i < m_sequence.steps.size(); ++i) {
        const auto& step = m_sequence.steps[i];

        std::string prefix = std::to_string(i + 1) + ". ";
        std::string suffix = " (" + std::to_string(
            (m_isEnteringDfu && i == m_index && m_stepRemaining >= 0)
                ? m_stepRemaining
                : ((i < m_index) || m_dfuSuccess)
                    ? 0
                    : step.duration
        ) + ")";

        const int first_line_width = std::max(1, steps_width - static_cast<int>(prefix.size()));
        const int continuation_width = std::max(1, steps_width - static_cast<int>(prefix.size()));

        StepBlock block;
        block.prefix_size = prefix.size();

        auto wrap_paragraph = [&](std::string paragraph) {
            while (!paragraph.empty() && (paragraph.front() == ' ' || paragraph.front() == '\t')) {
                paragraph.erase(paragraph.begin());
            }

            if (paragraph.empty()) {
                block.lines.push_back("");
                return;
            }

            size_t cursor = 0;
            while (cursor < paragraph.size()) {
                size_t line_end = cursor;
                size_t last_space = std::string::npos;
                const int target_width = block.lines.empty() ? first_line_width : continuation_width;

                while (line_end < paragraph.size() && static_cast<int>(line_end - cursor) < target_width) {
                    if (paragraph[line_end] == ' ') {
                        last_space = line_end;
                    }
                    ++line_end;
                }

                if (line_end < paragraph.size() && last_space != std::string::npos && last_space > cursor) {
                    line_end = last_space;
                }

                if (line_end == cursor) {
                    line_end = std::min(paragraph.size(), cursor + static_cast<size_t>(target_width));
                }

                block.lines.push_back(paragraph.substr(cursor, line_end - cursor));

                cursor = line_end;
                while (cursor < paragraph.size() && paragraph[cursor] == ' ') {
                    ++cursor;
                }
            }
        };

        std::string description = step.description;
        size_t paragraph_start = 0;
        while (paragraph_start <= description.size()) {
            size_t paragraph_end = description.find('\n', paragraph_start);
            if (paragraph_end == std::string::npos) {
                wrap_paragraph(description.substr(paragraph_start));
                break;
            }

            wrap_paragraph(description.substr(paragraph_start, paragraph_end - paragraph_start));
            paragraph_start = paragraph_end + 1;
        }

        if (block.lines.empty()) {
            block.lines.push_back("");
        }

        const size_t last_line_index = block.lines.size() - 1;
        block.lines[0] = prefix + block.lines[0] + (last_line_index == 0 ? suffix : "");

        for (size_t line_index = 1; line_index < block.lines.size(); ++line_index) {
            block.lines[line_index] = block.lines[line_index] + (line_index == last_line_index ? suffix : "");
        }

        for (const auto& line : block.lines) {
            block.width = std::max(block.width, line.size());
        }

        blocks.push_back(std::move(block));
    }

    size_t widest_block = 0;
    for (const auto& block : blocks) {
        widest_block = std::max(widest_block, block.width);
    }

    const int block_x = std::max(steps_x, right_edge - static_cast<int>(widest_block));
    int line_y = sy + 5;

    for (size_t i = 0; i < blocks.size(); ++i) {
        const int step_start_y = line_y;
        const auto& block = blocks[i];
        const bool is_current = m_isEnteringDfu && i == m_index;

        if (is_current) {
            attron(A_BOLD);
        } else {
            attron(A_DIM);
        }

        mvprintw(step_start_y, block_x, "%s", block.lines[0].c_str());

        for (size_t line_index = 1; line_index < block.lines.size(); ++line_index) {
            const int current_line_y = step_start_y + static_cast<int>(line_index);
            if (current_line_y >= sy + 20) {
                break;
            }

            mvprintw(current_line_y, block_x + static_cast<int>(block.prefix_size), "%s", block.lines[line_index].c_str());
        }

        line_y = step_start_y + static_cast<int>(block.lines.size());

        if (is_current) {
            attroff(A_BOLD);
        } else {
            attroff(A_DIM);
        }

        if (line_y >= sy + 20) {
            break;
        }
    }
}

void TuiDfuPanel::draw(int sy, int sx, int selected) {
    (void)selected;
    update_sequence_timer();

    const int panel_width = 80;
    const int button_x = sx + 2;
    const int button_y = sy + 5;
    const int button_width = kTuiDeviceCanvasWidth;
    const int button_height = 16;
    const int steps_x = sx + 40;
    const int steps_width = std::max(1, panel_width - 42);

    if (m_sequence.steps.empty()) {
        mvprintw(sy + 2, sx + 2, "No DFU helper sequence is available for this device.");
    } else if (m_dfuSuccess) {
        mvprintw(sy + 2, sx + 2, "Device entered DFU mode successfully.");
    } else {
        mvprintw(sy + 2, sx + 2, "Time to put the device into DFU mode. Locate the buttons as marked below on");
        mvprintw(sy + 3, sx + 2, "your device and check the instructions on the right.");
    }

    draw_wrapped_steps(sy, sx, steps_x, steps_width);
    draw_sequence_buttons(sy, sx, button_x, button_y, button_width, button_height);
}

void TuiDfuPanel::handle_enter(int selected, int sy, int sx) {
    switch(selected) {
        case 0: { // Back
            reboot();
            m_isEnteringDfu = false;
            m_dfuSuccess = false;
            GetFrame()->ShowMain(0);
            break;
        }
        case 1: { // Start
            start_sequence();
            break;
        }
    }
}

void TuiDfuPanel::handle_device_update(const DeviceState& state) {
    if (state.connected && !state.productType.empty()) {
        load_sequence_for_product(state.productType, false);
    }

    if (!state.connected) {
        if (!m_isEnteringDfu) {
            reset_sequence_state();
            GetFrame()->ShowMain(1);
        }
        return;
    }

    if (m_isEnteringDfu && state.mode == DeviceMode::DFU) {
        m_isEnteringDfu = false;
        m_dfuSuccess = true;
        m_waitingForDfuTransition = true;
        m_dfuSuccessAt = std::chrono::steady_clock::now();
        m_index = m_sequence.steps.size();
        m_stepRemaining = -1;

        GetFrame()->SetSelected(1);
        return;
    }

    if (m_isEnteringDfu && state.mode == DeviceMode::Normal) {
        reset_sequence_state();
        if (GetFrame()->GetRecoveryPanel()) {
            GetFrame()->GetRecoveryPanel()->SetStatusText("Hmm... It seems like the device didn't enter recovery mode.");
        }
        GetFrame()->ShowRecovery(0);
    }

    if (m_isEnteringDfu && state.mode == DeviceMode::Recovery) {
        reset_sequence_state();
        GetFrame()->ShowMain(0);
    }
}

void TuiDfuPanel::reboot() {
    std::thread([this]() {
        const auto state = GetFrame()->GetDeviceState();

        if (!state.connected || state.mode != DeviceMode::Recovery)
            return;

        irecv_client_t client = nullptr;
        int attempts = 0;
        const int max_attempts = 8;

        while (attempts < max_attempts) {
            if (irecv_open_with_ecid(&client, state.ecid) == IRECV_E_SUCCESS)
                break;
            attempts++;
            std::this_thread::sleep_for(std::chrono::milliseconds(250));
        }

        if (!client) return;
        irecv_setenv(client, "auto-boot", "true");
        irecv_saveenv(client);
        irecv_reboot(client);
        irecv_close(client);
    }).detach();
}

#endif // WITH_TUI

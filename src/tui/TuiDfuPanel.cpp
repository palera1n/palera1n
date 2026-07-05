#ifdef WITH_TUI

#include "TuiDfuPanel.hpp"

#include <algorithm>
#include <thread>
#include <chrono>
#include <unordered_set>

#include <ncurses.h>
#include <libirecovery.h>

#include "Tui.hpp"
#include "TuiDfuArt.hpp"
#include "TuiText.hpp"
#include "TuiRecoveryPanel.hpp"
#include "../sequence.hpp"
#include "../event.hpp"
#include "../globals.h"
#include "../paleinfo.h"

static const char *buttons[] = { "Back", "Start" };
static constexpr int kDfuPanelCoordinateWidth = 310;
static constexpr int kTuiDeviceCanvasWidth = 52;
static constexpr int kDfuButtonCoordinateMaxY = 200;
static constexpr int kDfuContentX = 2;
static constexpr int kDfuContentWidth = 60;
static constexpr int kDfuStepsMaxLines = 5;
static constexpr int kDfuStepsBottomY = 22;
static constexpr int kDfuButtonX = 2;
static constexpr int kDfuButtonY = 4;

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

void TuiDfuPanel::draw_sequence_buttons(int button_x, int button_y, int button_width, int button_height) const {
    if (m_sequence.buttons.empty()) {
        return;
    }

    std::unordered_set<std::string> active_buttons;
    if (m_isEnteringDfu && m_index < m_sequence.steps.size()) {
        const auto& buttons_for_step = m_sequence.steps[m_index].activeButtons;
        active_buttons.insert(buttons_for_step.begin(), buttons_for_step.end());
    }

    for (const auto& btn : m_sequence.buttons) {
        const int mapped_x = button_x + scale_coord(btn.x, kDfuPanelCoordinateWidth, button_width);
        const int mapped_y = button_y + scale_coord(btn.y, kDfuButtonCoordinateMaxY, button_height);

        const int label_x = std::clamp(mapped_x, button_x, button_x + std::max(0, button_width - 1));

        const bool is_active = active_buttons.find(btn.id) != active_buttons.end();
        if (is_active) {
            attron(A_BOLD);
        } else {
            attron(A_DIM);
        }

        mvprintw(mapped_y, label_x, "%s", btn.name.c_str());

        if (is_active) {
            attroff(A_BOLD);
        } else {
            attroff(A_DIM);
        }
    }
}

void TuiDfuPanel::draw_wrapped_steps(int steps_x, int steps_width, int max_lines, int bottom_y) const {
    if (steps_width <= 0 || max_lines <= 0 || m_sequence.steps.empty()) {
        return;
    }

    struct RenderedLine {
        bool is_current;
        bool is_completed;
        std::string text;
    };

    std::vector<RenderedLine> rendered_lines;
    rendered_lines.reserve(static_cast<size_t>(max_lines));

    for (size_t i = 0; i < m_sequence.steps.size(); ++i) {
        const auto& step = m_sequence.steps[i];
        const int seconds_left = (m_isEnteringDfu && i == m_index && m_stepRemaining >= 0)
            ? m_stepRemaining
            : ((i < m_index) || m_dfuSuccess) ? 0 : step.duration;

        std::string text = std::to_string(i + 1) + ". " + step.description + " (" + std::to_string(seconds_left) + ")";
        std::vector<std::string> wrapped;
        tui_text::append_wrapped_lines(text, steps_width, wrapped);

        const bool is_current = m_isEnteringDfu && i == m_index;
        const bool is_completed = (i < m_index) || m_dfuSuccess;
        for (const auto& line : wrapped) {
            if (static_cast<int>(rendered_lines.size()) >= max_lines) {
                break;
            }
            rendered_lines.push_back({ is_current, is_completed, line });
        }

        if (static_cast<int>(rendered_lines.size()) >= max_lines) {
            break;
        }
    }

    const int start_y = bottom_y - static_cast<int>(rendered_lines.size()) + 1;
    for (size_t i = 0; i < rendered_lines.size(); ++i) {
        const auto& line = rendered_lines[i];

        if (line.is_current) {
            attron(A_BOLD);
        } else if (line.is_completed) {
            attron(A_DIM);
        }

        mvprintw(start_y + static_cast<int>(i), steps_x, "%.*s", steps_width, line.text.c_str());

        if (line.is_current) {
            attroff(A_BOLD);
        } else if (line.is_completed) {
            attroff(A_DIM);
        }
    }
}

void TuiDfuPanel::draw(int sy, int sx, int selected) {
    (void)selected;
    update_sequence_timer();

    const int content_x = sx + kDfuContentX;

    if (m_sequence.steps.empty()) {
        mvprintw(sy + 2, content_x, "No DFU helper sequence is available for this device.");
    } else if (m_dfuSuccess) {
        mvprintw(sy + 2, content_x, "Device entered DFU mode successfully.");
    } else {
        tui_text::draw_wrapped_text(
            sy + 2,
            content_x,
            kDfuContentWidth,
            "Time to put the device into DFU mode.",
            2
        );
    }

    const int steps_x = content_x;
    const int steps_width = kDfuContentWidth;
    const int steps_max_lines = kDfuStepsMaxLines;
    const int steps_bottom_y = sy + kDfuStepsBottomY;

    const int button_x = sx + kDfuButtonX;
    const int button_y = sy + kDfuButtonY;
    const int button_width = kTuiDeviceCanvasWidth - 1;
    const int button_height = (steps_bottom_y - steps_max_lines + 1) - button_y;

    if (const DfuAsciiPreview* preview = find_dfu_ascii_preview(m_sequence.imageName)) {
        draw_dfu_ascii_preview(*preview, sx, sy);
    }

    draw_sequence_buttons(button_x, button_y, button_width, button_height);
    draw_wrapped_steps(steps_x, steps_width, steps_max_lines, steps_bottom_y);
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

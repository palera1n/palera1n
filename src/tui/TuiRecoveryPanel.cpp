#ifdef WITH_TUI

#include "TuiRecoveryPanel.hpp"

#include <ncurses.h>
#include <string>
#include <thread>
#include <chrono>

#include "Tui.hpp"
#include "TuiText.hpp"
#include "../globals.h"
#include "../paleinfo.h"

#include <libimobiledevice/libimobiledevice.h>
#include <libimobiledevice/lockdown.h>

static const char *buttons[] = { "Back", "Next" };
static constexpr int kRecoveryContentX = 2;
static constexpr int kRecoveryContentWidth = 60;

TuiRecoveryPanel::TuiRecoveryPanel(TuiFrame* frame)
    : TuiPanel(frame), m_status_text(""), m_is_entering_recovery(false), m_buttons_disabled(false) {}

void TuiRecoveryPanel::SetStatusText(const std::string& text) {
    m_status_text = text;
    m_preserve_status_on_show = true;
}

const char** TuiRecoveryPanel::get_buttons() const {
    return buttons;
}

bool TuiRecoveryPanel::is_button_enabled(int btn_idx) const {
    return !m_buttons_disabled;
}

void TuiRecoveryPanel::on_show() {
    if (!m_preserve_status_on_show) {
        m_status_text = "";
    }
    m_preserve_status_on_show = false;
    m_is_entering_recovery = false;
    m_buttons_disabled = false;
    GetFrame()->SetSelected(1);

    if (palerain_flags & palerain_option_quick) {
        m_status_text = "Entering recovery mode...";
        m_buttons_disabled = true;
        enter_recovery_mode();
    }
}

void TuiRecoveryPanel::draw(int sy, int sx, int selected) {
    const int content_x = sx + kRecoveryContentX;
    const std::string intro = "The device needs to be put into DFU mode to apply the jailbreak. This is a manual process and we will guide you through it.";
    tui_text::draw_wrapped_text(sy + 2, content_x, kRecoveryContentWidth, intro, 6);

    if (!m_status_text.empty()) {
        tui_text::draw_wrapped_text(sy + 9, content_x, kRecoveryContentWidth, m_status_text, 3);
    }
}

void TuiRecoveryPanel::handle_enter(int selected, int sy, int sx) {
    if (m_buttons_disabled) return;

    switch(selected) {
        case 0:
            GetFrame()->ShowMain(1);
            break;
        case 1:
            m_status_text = "Entering recovery mode...";
            m_buttons_disabled = true;
            enter_recovery_mode();
            break;
    }
}

void TuiRecoveryPanel::handle_device_update(const DeviceState& state) {
    if (m_is_entering_recovery && state.mode == DeviceMode::Recovery) {
        m_status_text = "Device is now in recovery mode.";
        std::this_thread::sleep_for(std::chrono::seconds(3));
        m_status_text = "";
        m_is_entering_recovery = false;
        m_buttons_disabled = false;
        GetFrame()->ShowDfu(1);
        return;
    }

    if (m_is_entering_recovery && state.connected && state.mode == DeviceMode::Normal) {
        m_is_entering_recovery = false;
        m_buttons_disabled = false;
        m_status_text = "Hmm... It seems like the device didn't enter recovery mode. Please try again.";
        return;
    }

    if (!m_is_entering_recovery && !state.connected) {
        m_buttons_disabled = false;
        GetFrame()->ShowMain(1);
        return;
    }
}

void TuiRecoveryPanel::enter_recovery_mode() {
    m_is_entering_recovery = true;
    m_buttons_disabled = true;

    std::thread([this]() {
        DeviceState state = GetFrame()->GetDeviceState();
        if (!state.connected) {
            m_is_entering_recovery = false;
            m_buttons_disabled = false;
            return;
        }

        idevice_t device = nullptr;
        lockdownd_client_t client = nullptr;

        if (idevice_new(&device, state.udid.c_str()) != IDEVICE_E_SUCCESS) {
            m_is_entering_recovery = false;
            m_buttons_disabled = false;
            return;
        }

        if (lockdownd_client_new_with_handshake(device, &client, "palera1n") != LOCKDOWN_E_SUCCESS) {
            idevice_free(device);
            m_is_entering_recovery = false;
            m_buttons_disabled = false;
            return;
        }

        lockdownd_enter_recovery(client);

        lockdownd_client_free(client);
        idevice_free(device);
    }).detach();
}

#endif // WITH_TUI

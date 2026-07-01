#ifdef WITH_TUI

#pragma once
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

#endif // WITH_TUI

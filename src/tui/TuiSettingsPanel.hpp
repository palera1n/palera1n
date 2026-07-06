#ifdef WITH_TUI

#ifndef TUI_SETTINGS_PANEL_HPP
#define TUI_SETTINGS_PANEL_HPP

#include "TuiPanel.hpp"

class TuiSettingsPanel : public TuiPanel {
public:
    explicit TuiSettingsPanel(TuiFrame* frame);
    ~TuiSettingsPanel() override = default;

    void draw(int sy, int sx, int selected) override;
    void handle_enter(int selected, int sy, int sx) override;
    void handle_device_update(const DeviceState& state) override;
    bool is_button_enabled(int btn_idx) const override;

    int get_total_items() const override { return 11; }
    int get_button_count() const override { return 1; }
    const char** get_buttons() const override;

private:
    void edit_boot_args(int start_y, int start_x);
};

#endif // TUI_SETTINGS_PANEL_HPP

#endif // WITH_TUI

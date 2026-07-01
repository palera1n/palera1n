#ifdef WITH_TUI

#pragma once
#include "TuiPanel.hpp"

class TuiSettingsPanel : public TuiPanel {
public:
    explicit TuiSettingsPanel(TuiFrame* frame);
    ~TuiSettingsPanel() override = default;

    void draw(int sy, int sx, int selected) override;
    void handle_enter(int selected, int sy, int sx) override;
    void handle_device_update(const DeviceState& state) override;

    int get_total_items() const override { return 6; }
    int get_button_count() const override { return 1; }
    const char** get_buttons() const override;

private:
    void edit_boot_args(int start_y, int start_x);
};

#endif // WITH_TUI

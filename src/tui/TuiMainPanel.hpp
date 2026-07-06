#ifdef WITH_TUI

#ifndef TUI_MAIN_PANEL_HPP
#define TUI_MAIN_PANEL_HPP

#include "TuiPanel.hpp"

class TuiMainPanel : public TuiPanel {
public:
    explicit TuiMainPanel(TuiFrame* frame);
    ~TuiMainPanel() override = default;

    void draw(int sy, int sx, int selected) override;
    void handle_enter(int selected, int sy, int sx) override;
    void handle_device_update(const DeviceState& state) override;
    bool is_button_enabled(int btn_idx) const override;

    int get_total_items() const override { return 4; }
    int get_button_count() const override { return 3; }
    const char** get_buttons() const override;
};

#endif // TUI_MAIN_PANEL_HPP

#endif // WITH_TUI

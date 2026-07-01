#ifdef WITH_TUI

#pragma once
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
    void draw_sequence_buttons(int sy, int sx, int button_x, int button_y, int button_width, int button_height) const;
    void draw_wrapped_steps(int sy, int sx, int steps_x, int steps_width) const;

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

#endif // WITH_TUI

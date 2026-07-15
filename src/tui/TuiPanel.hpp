#ifdef WITH_TUI

#ifndef TUI_PANEL_HPP
#define TUI_PANEL_HPP

#include <ncurses.h>

#include "../events/event.hpp"

class TuiFrame;

enum PanelState {
    PANEL_MAIN = 0,
    PANEL_OPTIONS,
    PANEL_START_1,
    PANEL_START_2,
    PANEL_FINAL,
    PANEL_COUNT
};

class TuiPanel {
public:
    explicit TuiPanel(TuiFrame* frame)
        : m_frame(frame) {}

    virtual ~TuiPanel() = default;

    virtual void on_show() {}
    virtual void update() {}

    virtual void draw(int sy, int sx, int selected) = 0;
    virtual void handle_enter(int selected, int sy, int sx) = 0;
    virtual void handle_device_update(const DeviceState& state) = 0;
    virtual bool is_button_enabled(int btn_idx) const { return true; }

    virtual int get_total_items() const = 0;
    virtual int get_button_count() const = 0;
    virtual const char** get_buttons() const = 0;
protected:
    TuiFrame* GetFrame() const {
        return m_frame;
    }
private:
    TuiFrame* m_frame = nullptr;
};

#endif // TUI_PANEL_HPP

#endif // WITH_TUI

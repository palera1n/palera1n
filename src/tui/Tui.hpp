#ifdef WITH_TUI

#ifndef TUI_HPP
#define TUI_HPP

#include <memory>
#include <mutex>
#include <vector>

#include "../events/event.hpp"
#include "TuiPanel.hpp"

class TuiPanel;
class TuiMainPanel;
class TuiSettingsPanel;
class TuiRecoveryPanel;
class TuiDfuPanel;
class TuiExploitPanel;

class TuiFrame {
public:
    TuiFrame();
    ~TuiFrame();

    void Run();

    void ShowMain(int selected = 1);
    void ShowSettings(int selected = 5);
    void ShowRecovery(int selected = 1);
    void ShowDfu(int selected = 1);
    void ShowExploit(int selected = 0);

    PanelState GetCurrentPanel() const;
    int GetSelected() const;
    void SetSelected(int selected);

    bool IsRunning() const;
    void Stop();

    DeviceState GetDeviceState() const;
    void UpdateDeviceState(const DeviceState& state);

    TuiMainPanel* GetMainPanel() const;
    TuiSettingsPanel* GetSettingsPanel() const;
    TuiRecoveryPanel* GetRecoveryPanel() const;
    TuiDfuPanel* GetDfuPanel() const;
    TuiExploitPanel* GetExploitPanel() const;

private:
    void InitPanels();
    void InitDeviceEventListeners();
    void ActivatePanelIfNeeded(PanelState previous_panel);
    void DrawUi();
    void HandleInput(int ch, int start_y, int start_x);

    PanelState m_currentPanel = PANEL_MAIN;
    int m_selected = 1;
    bool m_running = true;

    DeviceState m_device;
    mutable std::mutex m_deviceMutex;

    std::vector<std::unique_ptr<TuiPanel>> m_panels;
};

void ui_run(void);
void update_tui_device_state(const DeviceState* new_state);

#endif // TUI_HPP

#endif // WITH_TUI

#ifndef APPFRAME_H
#define APPFRAME_H

#ifdef WITH_GUI

#include <wx/wx.h>
#include "../event.hpp"

#include "MainPanel.hpp"
#include "SettingsPanel.hpp"
#include "RecoveryPanel.hpp"
#include "DfuPanel.hpp"
#include "ExploitPanel.hpp"

void send_device_state(MainFrame* frame, const DeviceState& state);

class MainFrame : public wxFrame
{
public:
    MainFrame();

    void ShowMain();
    void ShowSettings();
    void ShowRecovery();
    void ShowDfu();
    void ShowExploit();

    const DeviceState& GetDeviceState() const { return m_state; }
    void UpdateDeviceState(const DeviceState& state);

    MainPanel* GetMainPanel() const { return m_main; }
    SettingsPanel* GetSettingsPanel() const { return m_settings; }
    RecoveryPanel* GetRecoveryPanel() const { return m_recovery; }
    DfuPanel* GetDfuPanel() const { return m_dfu; }
    ExploitPanel* GetExploitPanel() const { return m_exploit; }
private:
    MainPanel* m_main = nullptr;
    SettingsPanel* m_settings = nullptr;
    RecoveryPanel* m_recovery = nullptr;
    DfuPanel* m_dfu = nullptr;
    ExploitPanel* m_exploit = nullptr;

    DeviceState m_state;
};

class PalerainApp : public wxApp {
public:
    bool OnInit() override;
};

#endif // WITH_GUI

#endif // APPFRAME_H

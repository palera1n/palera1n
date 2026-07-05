#ifndef MAINPANEL_H
#define MAINPANEL_H

#ifdef WITH_GUI

#include <wx/wx.h>
#include "DevicePanel.hpp"
#include "../event.hpp"

class MainPanel;
class MainPanel : public DevicePanel
{
public:
    explicit MainPanel(MainFrame* frame, wxWindow* parent);
    void SetDeviceState(const DeviceState& state) override;
private:
    wxStaticText* m_deviceTitle = nullptr;
    wxStaticText* m_deviceSubtitle = nullptr;
    wxButton* m_startButton = nullptr;
};

#endif // WITH_GUI

#endif // MAINPANEL_H

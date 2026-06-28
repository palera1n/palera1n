#ifndef RECOVERYPANEL_H
#define RECOVERYPANEL_H

#ifdef WITH_GUI

#include <wx/wx.h>
#include "DevicePanel.hpp"

class RecoveryPanel;
class RecoveryPanel : public DevicePanel
{
public:
    explicit RecoveryPanel(MainFrame* frame, wxWindow* parent);
    void SetDeviceState(const DeviceState& state) override;
    void SetStatusText(const wxString& text);
    void EnterRecoveryMode();
    void OnShow(wxShowEvent& event) override;
private:
    wxButton* m_backButton = nullptr;
    wxButton* m_nextButton = nullptr;
    wxStaticText* m_statusText = nullptr;
    wxTimer m_timer;
    bool m_isEnteringRecovery = false;
};


#endif // WITH_GUI

#endif // RECOVERYPANEL_H

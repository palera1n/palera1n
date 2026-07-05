#ifndef DFUPANEL_H
#define DFUPANEL_H

#ifdef WITH_GUI

#include <wx/wx.h>
#include "DevicePanel.hpp"
#include "../event.hpp"
#include "../sequence.hpp"

class DfuPanel;
class DfuPanel : public DevicePanel
{
public:
    explicit DfuPanel(MainFrame* frame, wxWindow* parent);
    void SetDeviceState(const DeviceState& state) override;
    void OnShow(wxShowEvent& event) override;
    void LoadDevice(const std::string& productType);
    void StartSequence(const DfuSequence& seq);
    void RunStep();
    void Reboot();
private:
    wxStaticText* m_headerText = nullptr;
    wxButton* m_backButton = nullptr;
    wxButton* m_startButton = nullptr;

    wxBoxSizer* m_stepsSizer = nullptr;

    std::vector<wxStaticText*> m_stepLabels;

    DfuSequence m_sequence;
    size_t m_index = 0;
    int m_stepRemaining = 0;

    wxTimer m_timer;
    wxTimer m_stagnentTimer;
    bool m_isEnteringDfu = false;

    wxPanel* m_devicePanel = nullptr;
    wxStaticBitmap* m_deviceImage = nullptr;
    std::vector<wxStaticText*> m_buttonLabels;
    size_t m_actionExecutedIndex = std::numeric_limits<size_t>::max();
};


#endif // WITH_GUI

#endif // DFUPANEL_H

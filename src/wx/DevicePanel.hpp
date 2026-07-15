#ifndef DEVICEPANEL_H
#define DEVICEPANEL_H

#include <wx/wx.h>
#include "../events/event.hpp"

class MainFrame;
class DevicePanel : public wxPanel
{
public:
    DevicePanel(MainFrame* frame, wxWindow* parent)
        : wxPanel(parent),
          m_frame(frame)
    {
    }

    virtual void SetDeviceState(const DeviceState& state)
    {
        m_deviceState = state;
    }

    virtual void OnShow(wxShowEvent& event)
    {
        event.Skip();
    }
protected:
    MainFrame* GetMainFrame() const
    {
        return m_frame;
    }

    const DeviceState& GetDeviceState() const
    {
        return m_deviceState;
    }
private:
    MainFrame* m_frame;
    DeviceState m_deviceState;
};

#endif // DEVICEPANEL_H

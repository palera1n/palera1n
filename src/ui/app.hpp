#ifdef WITH_GUI
#pragma once
#include <wx/wx.h>

class PalerainApp : public wxApp {
public:
    bool OnInit() override;
};

#endif // WITH_GUI

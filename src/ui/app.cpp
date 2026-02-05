#ifdef WITH_GUI
#include "app.hpp"
#include <string>
#include "../utils/constants.h"

bool PalerainApp::OnInit() {
    auto* frame = new wxFrame(
        nullptr,
        wxID_ANY,
        "palera1n - Version " + wxString(PALERAIN_VERSION),
        wxDefaultPosition,
        wxSize(480, 360),
        wxDEFAULT_FRAME_STYLE & ~(wxMAXIMIZE_BOX | wxRESIZE_BORDER)
    );

    new wxStaticText(frame, wxID_ANY, "my cute ui", wxPoint(10, 10));

    frame->Show(true);
    return true;
}

wxIMPLEMENT_APP_NO_MAIN(PalerainApp);
#endif // WITH_GUI

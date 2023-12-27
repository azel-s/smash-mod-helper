#pragma once

#include "../ModHandler.h"
#include <wx/wx.h>

class BatchSelection : public wxDialog
{
	ModHandler* mHandler;
    Settings settings;

    wxButton* base;
    wxButton* config;
    wxButton* prc;

public:
    BatchSelection
    (
        wxWindow* parent, wxWindowID id,
        const wxString& title,
        ModHandler* mHandler,
        Settings settings,
        const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxDefaultSize,
        long style = wxDEFAULT_DIALOG_STYLE & ~wxRESIZE_BORDER,
        const wxString& name = wxASCII_STR(wxDialogNameStr)
    );

    void onBasePressed(wxCommandEvent& evt);
    void onConfigPressed(wxCommandEvent& evt);
    void onPrcPressed(wxCommandEvent& evt);
    void onFinishPressed(wxCommandEvent& evt);
};


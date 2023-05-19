#pragma once
#include "../ModHandler.h"
#include <wx/wx.h>

class RenameSelection : public wxDialog
{
private:
    vector<wxTextCtrl*> names;
    ModHandler* mHandler;

public:
    RenameSelection(wxWindow* parent, wxWindowID id,
        const wxString& title,
        ModHandler* mHandler,
        const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxDefaultSize,
        long style = wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER,
        const wxString& name = wxASCII_STR(wxDialogNameStr));

    // Getters
    map<string, string> getNames();
};
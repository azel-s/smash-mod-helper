#pragma once
#include "../ModHandler.h"
#include <wx/wx.h>

class BaseSelection : public wxDialog
{
private:
    map<string, set<Slot>> slots;
    vector<wxChoice*> baseSlots;
    ModHandler* mHandler;
    Settings settings;

public:
    BaseSelection(wxWindow* parent, wxWindowID id,
        const wxString& title,
        ModHandler* mHandler,
        Settings settings,
        const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxDefaultSize,
        long style = wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER,
        const wxString& name = wxASCII_STR(wxDialogNameStr));

    // Getters
    map<string, map<Slot, set<Slot>>> getBaseSlots();
};
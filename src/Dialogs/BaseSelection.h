#pragma once
#include "../ModHandler.h"
#include <wx/wx.h>

class BaseSelection : public wxDialog
{
private:
    vector<wxChoice*> baseSlots;
    ModHandler* mHandler;

public:
    BaseSelection(wxWindow* parent, wxWindowID id,
        const wxString& title,
        ModHandler* mHandler,
        bool readPrevBase,
        const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxDefaultSize,
        long style = wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER,
        const wxString& name = wxASCII_STR(wxDialogNameStr));

    // Getters
    map<string, map<Slot, set<Slot>>> getBaseSlots();
};
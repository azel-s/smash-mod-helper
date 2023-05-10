#pragma once
#include "../ModHandler.h"
#include <wx/wx.h>
#include <wx/clrpicker.h>

class InkSelection : public wxDialog
{
    private:
        struct Add
        {
            wxButton* button;
            wxBoxSizer* sizer;
            wxChoice* list;
        };

        ModHandler* mHandler;

        vector<wxColourPickerCtrl*> finalEffects;
        vector<wxColourPickerCtrl*> finalArrows;
        vector<wxChoice*> finalSlots;

        Add add;
        wxArrayString list;

        wxBoxSizer* sizerM;
        wxBoxSizer* sizerH;
        wxBoxSizer* sizerV;

    public:
        InkSelection(wxWindow* parent, wxWindowID id,
            const wxString& title,
            ModHandler* mHandler,
            Settings settings,
            const wxPoint& pos = wxDefaultPosition,
            const wxSize& size = wxDefaultSize,
            long style = wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER,
            const wxString& name = wxASCII_STR(wxDialogNameStr));

        // Modifiers
        void onAddPressed(wxCommandEvent& evt);

        // Getters
        map<Slot, InklingColor> getFinalColors();
};
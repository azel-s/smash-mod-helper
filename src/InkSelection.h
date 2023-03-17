#pragma once
#include "SmashData.h"
#include <wx/wx.h>
#include <wx/spinctrl.h>
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

        vector<InklingColor> vanillaColors
        {
            InklingColor(wxColour(0.758027 * 255, 0.115859 * 255, 0.04 * 255), wxColour(0.92 * 255, 1 * 255, 0.1 * 255)),
            InklingColor(wxColour(0.04 * 255, 0.0608165 * 255, 0.758027 * 255), wxColour(0.1 * 255, 0.72 * 255, 1 * 255)),
            InklingColor(wxColour(0.79 * 255, 0.504014 * 255, 0.04 * 255), wxColour(1 * 255, 0.47 * 255, 0 * 255)),
            InklingColor(wxColour(0.347369 * 255, 0.582004 * 255, 0.04 * 255), wxColour(0.11 * 255, 1 * 255, 0 * 255)),
            InklingColor(wxColour(0.758027 * 255, 0.0608165 * 255, 0.273385 * 255), wxColour(1 * 255, 0 * 255, 0.38 * 255)),
            InklingColor(wxColour(0.04 * 255, 0.47948 * 255, 0.388556 * 255), wxColour(0 * 255, 0.4 * 255, 1 * 255)),
            InklingColor(wxColour(0.47948 * 255, 0.04 * 255, 0.582004 * 255), wxColour(1 * 255, 0 * 255, 0.283 * 255)),
            InklingColor(wxColour(0.04 * 255, 0.0462798 * 255, 0.114017 * 255), wxColour(0.25 * 255, 0.212 * 255, 0.556 * 255))
        };

        map<string, string> slotsMap;

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
            SmashData& mod,
            const wxPoint& pos = wxDefaultPosition,
            const wxSize& size = wxDefaultSize,
            long style = wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER,
            const wxString& name = wxASCII_STR(wxDialogNameStr));

        // Modifiers
        void onAddPressed(wxCommandEvent& evt);

        // Getters
        map<int, InklingColor> getFinalColors();
};
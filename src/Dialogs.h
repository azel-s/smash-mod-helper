#pragma once
#include "HelperStructures.h"
#include <wx/wx.h>
#include <wx/spinctrl.h>
#include <wx/clrpicker.h>
#include <vector>
#include <set>
#include <map>
using std::vector;
using std::set;
using std::map;

class BaseSlotsDialog : public wxDialog
{
    private:
        map<string, map<string, set<string>>> slots;
        vector<wxChoice*> baseSlots;
    
    public:
        BaseSlotsDialog(wxWindow* parent, wxWindowID id,
            const wxString& title,
            map<string, set<string>>& additionalSlots,
            unordered_map<string, string>& charNames,
            const wxPoint& pos = wxDefaultPosition,
            const wxSize& size = wxDefaultSize,
            long style = wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER,
            const wxString& name = wxASCII_STR(wxDialogNameStr));
    
        // Getters
        map<string, map<string, set<string>>> getBaseSlots(map<string, set<string>>& additionalSlots);
};

class prcxDialog : public wxDialog
{
    private:
        vector<wxSpinCtrl*> maxSlots;
        // char, slot, name-type, name
        map<string, map<string, wxName>> slotNames;
    
        /*
        size_t N = 20;
        size_t M = 20;
    
        std::vector<int> normal;
        normal.resize(N * M);
    
        for (size_t i = 0; i < N; ++i)
            for (size_t j = 0; j < M; ++j)
                normal[i + j * N] = j;
        */
    
    public:
        prcxDialog(wxWindow* parent, wxWindowID id,
            const wxString& title,
            map<string, set<string>>& allSlots,
            unordered_map<string, string>& charNames,
            bool slots,
            bool names,
            const wxPoint& pos = wxDefaultPosition,
            const wxSize& size = wxDefaultSize,
            long style = wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER,
            const wxString& name = wxASCII_STR(wxDialogNameStr));
    
        // Getters
        map<string, int> getMaxSlots(map<string, set<string>>& allSlots);
        map<string, map<int, Name>> getNames();
        map < string, map<int, string>> getAnnouncers();
};

class InklingDialog : public wxDialog
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
        InklingDialog(wxWindow* parent, wxWindowID id,
            const wxString& title,
            map<string, string>& slotsMap,
            map<int, InklingColor>& inklingColors,
            unordered_map<string, string>& charNames,
            const wxPoint& pos = wxDefaultPosition,
            const wxSize& size = wxDefaultSize,
            long style = wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER,
            const wxString& name = wxASCII_STR(wxDialogNameStr));

        // Modifiers
        void onAddPressed(wxCommandEvent& evt);
    
        // Getters
        map<int, InklingColor> getFinalColors();
};
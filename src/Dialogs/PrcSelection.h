#pragma once
#include "../ModHandler.h"
#include <wx/wx.h>
#include <wx/spinctrl.h>
#include <wx/clrpicker.h>

class PrcSelection : public wxDialog
{
private:
	vector<wxSpinCtrl*> maxSlots;
	// char, slot, name
	map<string, map<Slot, wxName>> slotNames;

public:
	PrcSelection(wxWindow* parent, wxWindowID id,
		const wxString& title,
		ModHandler& mHandler,
		bool readPrevNames,
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		long style = wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER,
		const wxString& name = wxASCII_STR(wxDialogNameStr));

	// Getters
	map<string, Slot> getMaxSlots(ModHandler* mHandler);
	map<string, map<Slot, Name>> getNames();
	map <string, map<Slot, string>> getAnnouncers();
};

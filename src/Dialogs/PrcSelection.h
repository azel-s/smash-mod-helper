#pragma once
#include "../ModHandler.h"
#include "../HelperStructures.h"
#include <wx/wx.h>

class PrcSelection : public wxDialog
{
private:
	ModHandler* mHandler;
	vector<wxSpinCtrl*> maxSlots;
	// char, slot, name
	map<string, map<Slot, wxName>> slotNames;

public:
	PrcSelection(wxWindow* parent, wxWindowID id,
		const wxString& title,
		ModHandler* mHandler,
		Settings settings,
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		long style = wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER,
		const wxString& name = wxASCII_STR(wxDialogNameStr));

	// Getters
	map<string, Slot> getMaxSlots();
	map<string, map<Slot, Name>> getNames();
	map <string, map<Slot, string>> getAnnouncers();
};

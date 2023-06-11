#pragma once
#include "../ModHandler.h"
#include "../HelperStructures.h"
#include <wx/wx.h>

class prcArgument : public wxObject
{
public:
	prcArgument(wxTextCtrl* text = nullptr) : text(text) {}

	wxTextCtrl* text;
};

class PrcSelection : public wxDialog
{
private:
	ModHandler* mHandler;
	vector<wxSpinCtrl*> maxSlots;
	map<string, set<Slot>> allSlots;
	// char, slot, name
	map<string, map<Slot, wxName>> slotNames;

public:
	PrcSelection
	(
		wxWindow* parent, wxWindowID id,
		const wxString& title,
		ModHandler* mHandler,
		Settings settings,
		map<string, map<int, CssData>>* css = nullptr,
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		long style = wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER,
		const wxString& name = wxASCII_STR(wxDialogNameStr)
	);

	// Functions
	void onType(wxCommandEvent& evt);
	void onLoadPressed(wxCommandEvent& evt);
	void onClosePressed(wxCommandEvent& evt);

	// Getters
	map<string, Slot> getMaxSlots();
	map<string, map<Slot, Name>> getNames();
	map<string, map<Slot, int>> getDB(string type);
	map <string, map<int, Name>> getAnnouncers();
};

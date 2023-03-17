#pragma once
#include "SmashData.h"
#include <wx/wx.h>
#include <wx/spinctrl.h>
#include <wx/clrpicker.h>

class PrcSelection : public wxDialog
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
	PrcSelection(wxWindow* parent, wxWindowID id,
		const wxString& title,
		SmashData& mod,
		bool readPrevNames,
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		long style = wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER,
		const wxString& name = wxASCII_STR(wxDialogNameStr));

	// Getters
	map<string, int> getMaxSlots(map<string, set<string>>& allSlots);
	map<string, map<int, Name>> getNames();
	map < string, map<int, string>> getAnnouncers();
};

#pragma once
#include "ModHandler.h"
#include "HelperStructures.h"
#include <wx/wx.h>
#include <wx/spinctrl.h>
using std::vector;

class App : public wxApp
{
public:
	bool OnInit();
};

class MainFrame : public wxFrame
{
private:
	/* --- GUI Parts --- */
	wxPanel* panel;
	wxMenuItem* inkMenu;
	wxMenuItem* deskMenu;
	wxBrowse browse;
	wxListBox* charsList;
	vector<wxCheckBox*> fileTypeBoxes;
	wxInitSlots initSlots;
	wxFinalSlots finalSlots;
	wxButtons buttons;
	wxTextCtrl* logWindow;
	wxLogTextCtrl* log;

	/* --- Data Variables --- */
	ModHandler mHandler;
	Settings settings;
	string iPath;

	/* --- HELPER FUNCTIONS --- */
	// Controls
	void updateControls(bool character = true, bool fileType = true, bool initSlot = true, bool finalSlot = true, bool newAddSlot = false, bool newInkSlot = false);

	// Settings
	void readSettings();
	void updateSettings();

	/* --- BOUND FUNCTIONS --- */
	void toggleSetting(wxCommandEvent& evt);
	void onBrowse(wxCommandEvent& evt);
	void onSelect(wxCommandEvent& evt);
	void onActionPressed(wxCommandEvent& evt);
	void onBatchPressed(wxCommandEvent& evt);
	void onLogPressed(wxCommandEvent& evt);
	void onBasePressed(wxCommandEvent& evt);
	void onConfigPressed(wxCommandEvent& evt);
	void onPrcPressed(wxCommandEvent& evt);
	void onInkPressed(wxCommandEvent& evt);
	void onDeskPressed(wxCommandEvent& evt);
	void onMenuClose(wxCommandEvent& evt);

public:
	/* --- GETTERS ---	*/
	wxArrayString getSelectedCharCodes();
	wxArrayString getSelectedFileTypes();

public:
	/* --- CONSTRUCTOR ---	*/
	MainFrame(const wxString& title);
};
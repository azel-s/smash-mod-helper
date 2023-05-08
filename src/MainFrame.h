#pragma once
#include "ModHandler.h"
#include "HelperStructures.h"
#include <wx/wx.h>
#include <wx/spinctrl.h>
using std::vector;

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
	VanillaHandler vHandler;
	Settings settings;
	string iPath;	// Initial Path

	/* --- Helpers ---	*/
	// ...
	void resetFileTypeBoxes();
	void resetButtons();

	// Settings
	void readSettings();
	void updateSettings();

	// Updaters/Modifiers
	void updateFileTypeBoxes();
	void updateButtons();
	void updateInkMenu();

	// Bind Functions
	void toggleBaseReading(wxCommandEvent& evt);
	void toggleNameReading(wxCommandEvent& evt);
	void toggleInkReading(wxCommandEvent& evt);
	void toggleSelectionType(wxCommandEvent& evt);
	void onBrowse(wxCommandEvent& evt);
	void onCharSelect(wxCommandEvent& evt);
	void onFileTypeSelect(wxCommandEvent& evt);
	void onModSlotSelect(wxCommandEvent& evt);
	void onUserSlotSelect(wxCommandEvent& evt);
	void onMovePressed(wxCommandEvent& evt);
	void onDuplicatePressed(wxCommandEvent& evt);
	void onDeletePressed(wxCommandEvent& evt);
	void onLogPressed(wxCommandEvent& evt);
	void onConfigPressed(wxCommandEvent& evt);
	void onBasePressed(wxCommandEvent& evt);
	void onInkPressed(wxCommandEvent& evt);
	void onDeskPressed(wxCommandEvent& evt);
	void onPrcPressed(wxCommandEvent& evt);
	void onMenuClose(wxCommandEvent& evt);
	void onClose(wxCloseEvent& evt);
	void onTestPressed(wxCommandEvent& evt);// Test Bound Function

public:
	/* --- Getters ---	*/
	wxArrayString getSelectedCharCodes();
	wxArrayString getSelectedFileTypes();
	bool isFileTypeSelected();

	// Destructor
	~MainFrame();

public:
	// Constructor
	MainFrame(const wxString& title);
};
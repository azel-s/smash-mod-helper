#pragma once
#include "SmashData.h"
#include "HelperStructures.h"
#include <wx/wx.h>
#include <wx/spinctrl.h>
using std::vector;

struct Settings
{
	bool prcxOutput = false;
	bool showLogWindow = false;

	bool readNames = true;
};

class MainFrame : public wxFrame
{
	private:
		// Data Variables
		SmashData data;

		// Settings
		Settings settings;

		// GUI Parts
		wxPanel* panel;
		wxMenuBar* menuBar;
		wxMenuItem* inkMenu;

		wxBrowse browse;
		wxListBox* charsList;
		wxCheckBox** fileTypeBoxes;
		wxInitSlots initSlots;
		wxFinalSlots finalSlots;
		wxButtons buttons;
		wxLogTextCtrl* log;
		wxTextCtrl* logWindow;
		wxStatusBar* statusBar;

		// Reset Helpers
		void resetFileTypeBoxes();
		void resetButtons();

		// Getters
		wxArrayString getSelectedFileTypes();
		bool isFileTypeSelected();

		// Test Function
		void test(wxCommandEvent& evt);
		map<string, map<string, Name>> readNames();
		map<string, map<string, string>> readBaseSlots();

		// Updaters/Modifiers
		void updateFileTypeBoxes();
		void updateButtons();
		void updateInkMenu();

		// Bind Functions
		void togglePRCOutput(wxCommandEvent& evt);
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
		void onPrcPressed(wxCommandEvent& evt);
		void onMenuClose(wxCommandEvent& evt);
		void onClose(wxCloseEvent& evt);

		// Destructor
		~MainFrame();

	public:
		// Constructor
		MainFrame(const wxString& title);
};
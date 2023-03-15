#pragma once
#include "SmashData.h"
#include "HelperStructures.h"
#include <wx/wx.h>
#include <wx/spinctrl.h>
using std::vector;

class MainFrame : public wxFrame
{
	private:
		// Data Variables
		SmashData data;

		// GUI Parts
		wxPanel* panel;

		wxBrowse browse;
		wxListBox* charsList;
		wxCheckBox** fileTypeBoxes;
		wxInitSlots initSlots;
		wxFinalSlots finalSlots;
		wxButtons buttons;
		wxLogTextCtrl* log;
		wxTextCtrl* logWindow;
		wxStatusBar* statusBar;

		// UI Setup Helper(s)
		void setupFileTypeBoxes(wxPanel* panel);

		// Reset Helpers
		void resetFileTypeBoxes();
		void resetButtons();

		// Getters
		wxArrayString getSelectedFileTypes();
		bool isFileTypeSelected();

		// Updaters/Modifiers
		void updateFileTypeBoxes();
		void updateButtons();

		// Bind Functions
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
		void onPrcxPressed(wxCommandEvent& evt);
		void onPrcxPressed2(wxCommandEvent& evt);
		void onClose(wxCloseEvent& evt);

		// Destructor
		~MainFrame();

	public:
		// Constructor
		MainFrame(const wxString& title);
};
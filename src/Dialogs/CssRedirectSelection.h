#pragma once
#include "../ModHandler.h"
#include <wx/wx.h>

struct Data
{
public:
	Slot left;
	Slot right;

	wxStaticText* text;
	wxTextCtrl* charcode;

	wxButton* subL;
	wxButton* subR;
	wxButton* addL;
	wxButton* addR;

	Data(Slot left = Slot(-1), Slot right = Slot(-1), wxStaticText* text = nullptr, wxTextCtrl* charcode = nullptr,
		wxButton* subL = nullptr, wxButton* subR = nullptr, wxButton* addL = nullptr, wxButton* addR = nullptr);

	void subLeft(int num = 1);
	void subRight(int num = 1);
	void addLeft(int num = 1);
	void addRight(int num = 1);

	void destroy();
};

struct Node
{
	Data* data;

	Node* next;
	Node* prev;

	Node(Data* data = nullptr, Node* prev = nullptr, Node* next = nullptr);

	static void deleteMid(Node* prev, Node* current, Node* next);
};

class wxCSSArgument : public wxObject
{
public:
	wxCSSArgument(string charcode = "", Node* node = nullptr, wxFlexGridSizer * sizer = nullptr) : charcode(charcode), node(node), sizer(sizer) {}

	string charcode;
	Node* node;
	wxFlexGridSizer* sizer;
};

class CssRedirectSelection : public wxDialog
{
private:
	map<string, Node*> data;
	ModHandler* mHandler;
	wxScrolled<wxPanel>* panel;

	// Bind
	void onSubL(wxCommandEvent& evt);
	void onSubR(wxCommandEvent& evt);
	void onAddL(wxCommandEvent& evt);
	void onAddR(wxCommandEvent& evt);

public:
	CssRedirectSelection(wxWindow* parent, wxWindowID id,
		const wxString& title,
		ModHandler* mHandler,
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		long style = wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER,
		const wxString& name = wxASCII_STR(wxDialogNameStr));

	// Getters
	map<string, map<int, CssData>> getChanges();
	map<string, map<string, map<Slot, Slot>>> getRedirects();
};
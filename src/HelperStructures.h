#pragma once
#include <wx/wx.h>
#include <wx/spinctrl.h>
#include <string>
#include <locale>
using namespace std;

struct Settings
{
	bool prcxOutput = false;
	bool showLogWindow = false;

	bool readBase = true;
	bool readNames = true;
	bool readInk = true;
};

struct wxBrowse
{
	wxButton* button;
	wxTextCtrl* text;
};

struct wxInitSlots
{
	wxChoice* list;
	wxStaticText* text;
};

struct wxFinalSlots
{
	wxSpinCtrl* list;
	wxStaticText* text;
};

struct wxButtons
{
	wxButton* mov;
	wxButton* dup;
	wxButton* del;

	wxButton* log;
	wxButton* base;
	wxButton* config;
	wxButton* prc;
};

struct Name
{
	wxString cssName;
	wxString cspName;
	wxString vsName;
	wxString stageName;
	wxString announcer;
};

struct wxName
{
	wxTextCtrl* cssName;
	wxTextCtrl* cspName;
	wxTextCtrl* vsName;
	wxTextCtrl* stageName;
	wxTextCtrl* announcer;
};

struct InklingColor
{
	wxColour effect;
	wxColour arrow;

	InklingColor()
	{
		effect.Set(0, 0, 0);
		arrow.Set(0, 0, 0);
	}

	InklingColor(wxColour effect, wxColour arrow)
	{
		this->effect.SetRGB(effect.GetRGB());
		this->arrow.SetRGB(arrow.GetRGB());
	}
};
#pragma once
#include <wx/wx.h>
#include <wx/spinctrl.h>
#include <string>
#include <set>
#include <map>
#include <vector>
#include <locale>
using namespace std;

class Slot
{
private:
	int slot;

public:
	Slot();
	Slot(int slot);
	Slot(string slot);

	int getInt() const;
	string getString() const;

	void set(Slot slot);
	void set(int slot);
	void set(string slot);

	bool operator <(const Slot& rhs) const;
};

class Path
{
private:
	string path;
	string type;
	Slot slot;
public:
	Path(string path = "");

	string getPath() const;
	string getType() const;
	Slot getSlot() const;

	void setSlot(Slot slot);
	void setSlot(int slot);
	void setSlot(string slot);

	bool operator <(const Path& rhs) const;
};

struct DBData
{
	int nIndex;
	int cIndex;
	int cGroup;

	string label;
	string article;

	// Constructors
	DBData();
	DBData(string code);
};

struct Config
{
	// files
	vector<string> newDirInfos;
	// files
	vector<string> newDirInfosBase;
	// code, slot, base-file, files
	map<string, map<Slot, map<string, set<string>>>> shareToVanilla;
	// code, slot, base-file, files
	map<string, map<Slot, map<string, set<string>>>> shareToAdded;
	// code, slot, section-label, files
	map<string, map<Slot, map<string, set<string>>>> newDirFiles;
};

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
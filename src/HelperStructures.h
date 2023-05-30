#pragma once
#include <wx/wx.h>
#include <wx/spinctrl.h>
#include <vector>
#include <string>
#include <map>
#include <set>
using namespace std;

class wxArgument : public wxObject
{
public:
	wxArgument(string str = "", int num = 0) : str(str), num(num) {}

	string str;
	int num;
};

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

struct Settings
{
	bool preview = true;
	bool baseSource = true;
	bool selectionType = false;

	bool readBase = true;
	bool readNames = true;
	bool readInk = true;
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
	wxButton* prcxml;
};

struct wxPreview
{
	wxStaticBitmap* chara_1;
	wxStaticBitmap* chara_2;
	wxStaticBitmap* chara_4;
	wxStaticBitmap* chara_7;
};

struct Name
{
	wxString cssName;
	wxString cspName;
	wxString vsName;
	wxString stageName;
	wxString announcer;
	wxString article;
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

	InklingColor() : effect(0, 0, 0), arrow(0, 0, 0) {}
	InklingColor(wxColour effect, wxColour arrow) : effect(effect), arrow(arrow) {}
};
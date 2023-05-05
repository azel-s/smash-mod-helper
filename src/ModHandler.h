#pragma once

#include "VanillaHandler.h"
#include <wx/wx.h>
#include <set>
#include <unordered_set>
#include <map>
#include <unordered_map>
#include <thread>
using json = nlohmann::json;
using std::vector, std::set, std::unordered_set, std::map, std::unordered_map;

class ModHandler
{
private:
	string path; // Mod Path
	vector<string> fileTypes; // Supported file types

	// Access Order: code, filetype, slot, files
	unordered_map<string, unordered_map<string, map<Slot, set<Path>>>> files;
	// Access Order: code, final-slot, base-slot,
	unordered_map<string, map<Slot, Slot>> slots;

	VanillaHandler vHandler;
	wxLogTextCtrl* log;

	bool debug;

	/* --- HELPERS (UNIVERSAL) --- */
	void addFile(string code, string fileType, int slot, string file);

	/* --- HELPERS (WX) --- */
	void wxLog(string message, bool debug = true);

public:
	/* --- TEST FUNCTIONS (WIP/DEBUG) --- */
	void test();

	/* --- CONSTRUCTORS --- */
	ModHandler(wxLogTextCtrl* log = nullptr);

	/* --- SETTERS (UNIVERSAL) --- */
	void setSlots(map<string, map<Slot, set<Slot>>> slots);
	void setDebug(bool debug);

	/* --- SETTERS (WX) --- */
	void wxSetLog(wxLogTextCtrl* log);

	/* --- GETTERS (UNIVERSAL) --- */
	string getName(string code);
	string getCode(string name);
	int getNumCharacters();
	set<Slot> getAddSlots(string code)  const;
	map<string, set<Slot>> getAddSlots() const;
	Slot getBaseSlot(string code, Slot slot) const;

	/* --- GETTERS (WX) --- */
	wxArrayString wxGetCharacterNames(string fileType = "") const;
	wxArrayString wxGetFileTypes(string code = "") const;
	wxArrayString wxGetSlots(string code, wxArrayString fileTypes = {}, bool findInAll = false) const;

	/* --- VERIFIERS (UNIVERSAL) ---*/
	bool hasChar(string code = "") const;
	bool hasFileType(string fileType = "") const;
	bool hasAddSlot(string code = "") const;

	/* --- VERIFIERS (WX) --- */
	bool wxHasSlot(string code, Slot slot, wxArrayString fileTypes = {}, bool findInAll = false) const;

	/* --- FUNCTIONS (UNIVERSAL) --- */
	void readFiles(string path);
	void removeDesktopINI();

	/* --- FUNCTIONS (WX) --- */
	void adjustFiles(string action, string code, wxArrayString fileTypes, Slot iSlot, Slot fSlot);
	void adjustFiles(string action, wxArrayString codes, wxArrayString fileTypes, Slot iSlot, Slot fSlot);

	/* --- FUNCTIONS (CONFIG/PRC) --- */
	void getNewDirSlots
	(
		vector<string>& newDirInfos,
		vector<string>& newDirInfosBase,
		map<string, map<Slot, map<string, set<string>>>>& shareToVanilla,
		map<string, map<Slot, map<string, set<string>>>>& shareToAdded,
		map<string, map<Slot, map<string, set<string>>>>& newDirFiles
	);
	bool createConfig();

	// Config & PRCX
	void patchXMLSlots(map<string, int>& maxSlots);
	void patchXMLNames(map<string, map<int, Name>>& names);
	void patchXMLAnnouncer(map<string, map<int, string>>& announcers);
	void patchXMLInkColors(map<int, InklingColor>& inklingColors);
	void createPRCXML(map<string, map<int, Name>>& names, map<string, map<int, string>>& announcers, map<string, int>& maxSlots);
	void createInkPRCXML(map<int, InklingColor>& inklingColors);
	void outputUTF(wofstream& file, wxString str, bool parse = false);

	// Readers
	map<int, InklingColor> readInk();
	map<string, map<Slot, Slot>> readBaseSlots();
	map<string, map<string, Name>> readNames();

	// Clear Mod Data
	void clear();
};
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

	// Access Order: charcode, filetype, slot, files
	unordered_map<string, unordered_map<string, map<int, set<string>>>> files;
	// Access Order: charcode, base-slot, final-slot
	map<string, map<int, set<int>>> slots;

	VanillaHandler vHandler;
	wxLogTextCtrl* log;

	// Helpers
	string convertPath(string input);
	void addFile(string charcode, string fileType, int slot, string file);

public:
	// Constructor(s)
	ModHandler(wxLogTextCtrl* log);

	// Read Data
	void readFiles(string path);

	// Mod Getters (wx)
	wxArrayString getCharacters() const;
	wxArrayString getFileTypes(string charcode) const;
	wxArrayString getSlots(string charcode) const;
	wxArrayString getSlots(string charcode, string fileType) const;
	wxArrayString getSlots(string charcode, wxArrayString fileTypes) const;

	string getBaseSlot(string charcode, string addSlot);
	// Make character-slots map
	map<string, set<string>> getAllSlots(string charcode = "all");

	// Mod Verifiers
	// Returns true if the fileType exists in any character
	bool hasFileType(string fileType) const;
	// Returns true if additional exists in any character
	bool hasAdditionalSlot() const;
	// Returns true if additional exists in specified character
	bool hasAdditionalSlot(string charcode) const;
	// Returns true if any of the fileTypes have the slot
	bool hasSlot(string charcode, string slot) const;
	// Returns true if any of the given fileTypes have the slot
	bool hasSlot(string charcode, wxArrayString fileTypes, string slot) const;

	// Mod Modifiers
	void adjustFiles(string action, string charcode, wxArrayString fileTypes, string initSlot, string finalSlot);
	void removeDesktopINI();

	// Config Getters
	map<string, set<string>> getAddSlots();
	void getNewDirSlots
	(
		const map<string, map<string, set<string>>>& baseSlots,
		vector<string>& newDirInfos,
		vector<string>& newDirInfosBase,
		map<string, map<string, map<string, set<string>>>>& shareToVanilla,
		map<string, map<string, map<string, set<string>>>>& shareToAdded,
		map<string, map<string, map<string, set<string>>>>& newDirFiles
	);

	// Config & PRCX
	void createConfig(map<string, map<string, set<string>>>& baseSlots);
	void patchXMLSlots(map<string, int>& maxSlots);
	void patchXMLNames(map<string, map<int, Name>>& names);
	void patchXMLAnnouncer(map<string, map<int, string>>& announcers);
	void patchXMLInkColors(map<int, InklingColor>& inklingColors);
	void createPRCXML(map<string, map<int, Name>>& names, map<string, map<int, string>>& announcers, map<string, int>& maxSlots);
	void createInkPRCXML(map<int, InklingColor>& inklingColors);
	void outputUTF(wofstream& file, wxString str, bool parse = false);

	// Readers
	map<int, InklingColor> readInk();
	map<string, map<string, int>> readBaseSlots();
	map<string, map<string, Name>> readNames();

	// Clear Mod Data
	void clear();
};
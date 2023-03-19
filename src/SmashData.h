#pragma once
#include "HelperStructures.h"
#include <wx/wx.h>
#include <vector>
#include <set>
#include <unordered_set>
#include <map>
#include <unordered_map>
#include <thread>
using std::vector, std::set, std::unordered_set, std::map, std::unordered_map;

struct SmashData
{
	// Log
	wxLogTextCtrl* log;

	// Mod Path
	string rootPath;
	// Currently supported filetypes
	vector<string> fileTypes;

	// Access Order: charcode, filetype, slot, files
	unordered_map<string, map<string, map<string, unordered_set<string>>>> mod;

	// Access Order: charcode, charname
	unordered_map<string, string> charNames;
	// Access Order: charcode, ID
	unordered_map<string, int> charIDFromcode;
	// Access Order: charcode, ID
	unordered_map<string, int> charcodeFromID;
	// Access Order: charname, charcode
	unordered_map<string, string> charCodes;
	// Access Order: charcode, files
	unordered_map<string, unordered_set<string>> vanillaEffectFiles;
	// Access Order: charcode, slot, files
	unordered_map<string, unordered_map<string, unordered_set<string>>> vanillaFiles;
	// Access Order: charcode, slot, files
	unordered_map<string, unordered_map<string, unordered_set<string>>> vanillaResultFiles;
	// Access Order: charcode, slot, files
	unordered_map<string, unordered_map<string, unordered_set<string>>> vanillaKirbyFiles;
	// Access Order: charcode, slot, files
	unordered_map<string, unordered_map<string, unordered_set<string>>> addedFiles;
	// Access Order: charcode, slot, files
	unordered_map<string, unordered_map<string, unordered_set<string>>> addedResultFiles;
	// Access Order: charcode, slot, files
	unordered_map<string, unordered_map<string, unordered_set<string>>> addedKirbyFiles;
	// Access Order: charcode, slot, files
	unordered_map<string, unordered_map<string, unordered_set<string>>> cameraFiles;

	// Access Order: charcode, base-slot, additional slots
	map<string, map<string, set<string>>> baseSlots;

	// Background thread
	std::thread vanillaThread;
	std::atomic<bool> vanillaThreadActive;
	std::atomic<bool> stopVanillaThread;

	// Constructor(s)
	SmashData();

	// Path Converters
	string convertPath(string input);

	// Read Data
	void readVanillaFiles(string fileType);
	void readData(string wxPath);

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
	void addData(string charcode, string fileType, string slot, string file);
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
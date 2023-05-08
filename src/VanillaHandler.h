#pragma once

#include "nlohmann/json.hpp"
#include "HelperStructures.h"
#include <unordered_map>
#include <vector>
#include <string>
#include <set>
using json = nlohmann::json;
using std::string, std::vector, std::unordered_map, std::set;

class VanillaHandler
{
private:
	// Vanilla data
	json vJson;
	bool okay;

	// Key: Character code, Value: Character name
	unordered_map<string, string*> code_name;
	// Key: Character name, Value: Character code
	unordered_map<string, string*> name_code;

	// Key: Character code, Value: Set(Path)
	unordered_map<string, set<Path>> effectFiles;

	// Key: Character code, Value: (Key: Slot, Value: (First: Announcer, Second: Announcer Article)
	unordered_map <string, map<Slot, DBData>> XML;

	// Index: slot, Value: { wxColour effect, wxColour arrow }
	vector<InklingColor> inklingColors;

	/* --- Helper Functions --- */
	void insertFiles(const json& tJson, map<string, set<Path>>& files, string type) const;
	void insertCodeName(string code, string name);

public:
	/* --- Constructor/Destructor --- */
	VanillaHandler(string filePath = "Files/");
	// INFO: Deletes dynamic strings for code/name.
	~VanillaHandler();

	/* --- Getters --- */
	// Character code/name
	string getName(string code) const;
	string getCode(string name) const;
	
	// JSON (and effect.data) Info
	int getFiles(string code, Slot slot, map<string, set<Path>>& files) const;

	// ui_chara_db Info
	DBData getXMLData(string code, Slot slot) const;

	// Inkling
	InklingColor getInklingColor(Slot slot) const;
	vector<InklingColor> getInklingColors() const;

	/* --- Verifiers --- */
	bool isOkay() const;
};
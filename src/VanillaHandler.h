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

	// Index: slot, Value: { wxColour effect, wxColour arrow }
	vector<InklingColor> inklingColors;

	// Helper Functions
	void insertFiles(const json& tJson, map<string, set<Path>>& files, string type) const;

public:
	/* --- Constructor/Destructor --- */
	// INFO: Path must end with '/'
	VanillaHandler(string filePath = "Files/");
	// INFO: Deletes dynamic strings for code/name.
	~VanillaHandler();

	// Setters/Modifiers
	void insertCodeName(string code, string name);

	/* --- Getters --- */
	// Character code/name
	string getCharName(string code) const;
	string getCharCode(string name) const;
	
	// JSON Info
	int getFiles(string code, Slot slot, map<string, set<Path>>& files) const;
	int getFiles(string code, int slot, map<string, set<Path>>& files) const;

	// Inkling
	InklingColor getInklingColor(Slot slot) const;
	vector<InklingColor> getInklingColors() const;

	/* --- Verifiers --- */
	bool isOkay() const;
};
#pragma once

#include "nlohmann/json.hpp"
#include "HelperStructures.h"
#include <unordered_map>
#include <vector>
#include <string>
using json = nlohmann::json;
using std::string, std::vector, std::unordered_map;

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

public:
	// Constructor/Destructor
	// INFO: Path must end with '/'
	VanillaHandler(string filePath = "Files/");
	~VanillaHandler();

	// Setters
	void insertCodeName(string code, string name);

	// Getters
	string getName(string code) const;
	string getName(int id) const;
	string getCode(string name) const;
	string getCode(int id) const;
	int getID(string code) const;
	InklingColor getInklingColor(int slot) const;
	vector<InklingColor> getInklingColors() const;
};
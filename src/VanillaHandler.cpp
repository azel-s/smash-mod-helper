#include "VanillaHandler.h"
#include <filesystem>
#include <fstream>
#include <string>
#include <queue>
namespace fs = std::filesystem;
using std::string, std::queue, std::ifstream;

VanillaHandler::VanillaHandler(string filePath)
{
	// Read Vanilla JSON
	ifstream jFile(filePath + "vanilla.json");
	if (jFile.is_open())
	{
		this->vJson = json::parse(jFile);
		okay = true;
	}
	else
	{
		okay = false;
	}
	jFile.close();

	// Read names
	ifstream nFile(filePath + "names.data");
	string code;
	string name;
	while (!nFile.eof())
	{
		// Code
		getline(nFile, code, ',');

		// Name
		getline(nFile, name);

		this->insertCodeName(code, name);
	}
	nFile.close();

	// Initialize inkling colors
	inklingColors.push_back(InklingColor(wxColour(0.758027 * 255, 0.115859 * 255, 0.04 * 255), wxColour(0.92 * 255, 1 * 255, 0.1 * 255)));
	inklingColors.push_back(InklingColor(wxColour(0.04 * 255, 0.0608165 * 255, 0.758027 * 255), wxColour(0.1 * 255, 0.72 * 255, 1 * 255)));
	inklingColors.push_back(InklingColor(wxColour(0.79 * 255, 0.504014 * 255, 0.04 * 255), wxColour(1 * 255, 0.47 * 255, 0 * 255)));
	inklingColors.push_back(InklingColor(wxColour(0.347369 * 255, 0.582004 * 255, 0.04 * 255), wxColour(0.11 * 255, 1 * 255, 0 * 255)));
	inklingColors.push_back(InklingColor(wxColour(0.758027 * 255, 0.0608165 * 255, 0.273385 * 255), wxColour(1 * 255, 0 * 255, 0.38 * 255)));
	inklingColors.push_back(InklingColor(wxColour(0.04 * 255, 0.47948 * 255, 0.388556 * 255), wxColour(0 * 255, 0.4 * 255, 1 * 255)));
	inklingColors.push_back(InklingColor(wxColour(0.47948 * 255, 0.04 * 255, 0.582004 * 255), wxColour(1 * 255, 0 * 255, 0.283 * 255)));
	inklingColors.push_back(InklingColor(wxColour(0.04 * 255, 0.0462798 * 255, 0.114017 * 255), wxColour(0.25 * 255, 0.212 * 255, 0.556 * 255)));
}

VanillaHandler::~VanillaHandler()
{
	for (auto i = code_name.begin(); i != code_name.end(); i++)
	{
		delete i->second;
	}

	for (auto i = name_code.begin(); i != name_code.end(); i++)
	{
		delete i->second;
	}
}

void VanillaHandler::insertCodeName(string code, string name)
{
	auto namePtr = new string(name);
	auto codePtr = new string(code);

	code_name[code] = namePtr;
	name_code[name] = codePtr;
}

string VanillaHandler::getName(string code) const
{
	auto iter = code_name.find(code);

	if (iter != code_name.end())
	{
		return *iter->second;
	}
	else
	{
		return "";
	}
}

string VanillaHandler::getName(int id) const
{
	auto iter = id_code.find(id);

	if (iter != id_code.end())
	{
		auto jter = code_name.find(*iter->second);

		if (jter != code_name.end())
		{
			return *jter->second;
		}
	}

	return "";
}

string VanillaHandler::getCode(string name) const
{
	auto iter = name_code.find(name);

	if (iter != name_code.end())
	{
		return *iter->second;
	}
	else
	{
		return "";
	}
}

string VanillaHandler::getCode(int id) const
{
	auto iter = id_code.find(id);

	if (iter != id_code.end())
	{
		return *iter->second;
	}
	else
	{
		return "";
	}
}

int VanillaHandler::getID(string code) const
{
	auto iter = code_id.find(code);

	if (iter != code_id.end())
	{
		return *iter->second;
	}
	else
	{
		return -1;
	}
}

InklingColor VanillaHandler::getInklingColor(int slot) const
{
	if (slot >= 0 && slot < 8)
	{
		return inklingColors[slot];
	}
	else
	{
		return InklingColor();
	}
}

vector<InklingColor> VanillaHandler::getInklingColors() const
{
	return inklingColors;
}
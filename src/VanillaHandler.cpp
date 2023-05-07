#include "VanillaHandler.h"
#include <filesystem>
#include <fstream>
#include <string>
#include <queue>
namespace fs = std::filesystem;
using std::string, std::queue, std::ifstream;

VanillaHandler::VanillaHandler(string filePath)
{
	if (!filePath.empty() && filePath[filePath.size() - 1] != '/')
	{
		filePath += "/";
	}

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
	if (nFile.is_open())
	{
		string code;
		string name;
		while (!nFile.eof())
		{
			// Code
			getline(nFile, code, ',');

			// Name
			getline(nFile, name, ' ');
			getline(nFile, name);

			insertCodeName(code, name);

			// Create DBData for each name
			for (int i = 0; i < 8; i++)
			{
				XML[code][Slot(i)] = DBData(code);
			}
		}
	}
	nFile.close();

	// Read DB
	ifstream dFile(filePath + "db.data");
	if (dFile.is_open())
	{
		string line;
		while (!dFile.eof())
		{
			string code;
			getline(dFile, code, ' ');

			getline(dFile, line, ' ');
			int slot = stoi(line);

			string type;
			getline(dFile, type, ' ');

			getline(dFile, line);

			if (type == "cIndex")
			{
				XML[code][Slot(slot)].cIndex = stoi(line);
			}
			else if (type == "cGroup")
			{
				XML[code][Slot(slot)].cGroup = stoi(line);
			}
			else if (type == "nIndex")
			{
				XML[code][Slot(slot)].nIndex = stoi(line);
			}
			else if (type == "label")
			{
				XML[code][Slot(slot)].label = line;
			}
			else if (type == "article")
			{
				XML[code][Slot(slot)].article = line;
			}
		}
	}
	dFile.close();

	// Read effect
	ifstream eFile(filePath + "effect.data");
	if (eFile.is_open())
	{
		string line;
		while (!eFile.eof())
		{
			getline(eFile, line);

			// effect/fighter/[charcode]/...
			// effect/fighter/ = 15
			effectFiles[line.substr(15, line.substr(15).find('/'))].insert(line);
		}
	}
	eFile.close();

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

string VanillaHandler::getCharName(string code) const
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

// @INPUT: Character's codename.
// @RETURN: Character's actual name (or empty if not found).
string VanillaHandler::getCharCode(string name) const
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

void VanillaHandler::insertFiles(const json& tJson, map<string, set<Path>>& files, string type) const
{
	auto iterFiles = tJson.find("files");

	if (iterFiles != tJson.end())
	{
		for (int i = 0; i < iterFiles->size(); i++)
		{
			Path path = Path(vJson["file_array"][(int)iterFiles->at(i)]);

			if (type != "kirbycopy" || path.getPath().find("/copy_") != string::npos)
			{
				files[type].insert(path);
			}
		}
	}

	for (auto i = tJson.begin(); i != tJson.end(); i++)
	{
		if (i.key() != "files")
		{
			insertFiles(*i, files, type);
		}
	}
}
// @INPUT:	Character code, slot, and map to write data to.
// @RETURN:	 0: Successfully wrote information
//			-1: Failed to find required folder/files
//			-2: Unknown error
int VanillaHandler::getFiles(string code, Slot slot, map<string, set<Path>>& files) const
{
	int result = 0;

	try
	{
		auto charIter = vJson["dirs"]["directories"]["fighter"]["directories"].find(code);
		if (charIter != vJson["dirs"]["directories"]["fighter"]["directories"].end())
		{
			auto iter = charIter->find("directories");
			if (iter != charIter->end())
			{
				auto slotIter = iter->find("c" + slot.getString());

				// Fighter (aka Vanilla or Added) & Camera & CMN Files
				if (slotIter != iter->end())
				{
					// Fighter files.
					auto filesIter = slotIter->find("files");
					if (filesIter != slotIter->end())
					{
						for (int i = 0; i < (*filesIter).size(); i++)
						{
							string file = vJson["file_array"][(int)filesIter->at(i)];

							if (file.find("/motion/") != string::npos)
							{
								files["added"].insert(Path(file));
							}
							else
							{
								files["vanilla"].insert(Path(file));
							}
						}
					}

					// Camera and CMN Files
					auto dirsIter = slotIter->find("directories");
					if (dirsIter != slotIter->end())
					{
						auto tempIter = dirsIter->find("camera");
						if (tempIter != dirsIter->end())
						{
							insertFiles(*tempIter, files, "camera");
						}

						tempIter = dirsIter->find("cmn");
						if (tempIter != dirsIter->end())
						{
							insertFiles(*tempIter, files, "cmn");
						}
					}
				}
				else
				{
					result = -1;
				}

				vector<string> types = { "append", "kirbycopy", "movie", "result" };
				for (auto& type : types)
				{
					auto tempIter = iter->find(type);
					if (tempIter != iter->end())
					{
						auto tempIter2 = tempIter->find("directories");
						if (tempIter2 != tempIter->end())
						{
							auto tempIter3 = tempIter2->find("c" + slot.getString());
							if (tempIter3 != tempIter2->end())
							{
								insertFiles(*tempIter3, files, type);
							}
						}
					}
					else if(code != "element")
					{
						return -1;
					}
				}

				// FinalSmash files.
				auto fsIter = iter->find("finalsmash");
				if (fsIter != iter->end())
				{
					insertFiles(*fsIter, files, "finalsmash");
				}
				else if (code != "element")
				{
					return -1;
				}
			}
			else
			{
				result = -1;
			}
		}
		else
		{
			result = -1;
		}

		auto charJter = effectFiles.find(code);
		if (charJter != effectFiles.end())
		{
			for (auto i = charJter->second.begin(); i != charJter->second.end(); i++)
			{
				files["effect"].insert(Path(*i));
			}
		}
		else if (code != "element")
		{
			return -1;
		}
	}
	catch (...)
	{
		result = -2;
	}

	return result;
}

DBData VanillaHandler::getXMLData(string code, Slot slot) const
{
	auto charIter = XML.find(code);
	if (charIter != XML.end())
	{
		auto slotIter = charIter->second.find(slot);
		if (slotIter != charIter->second.end())
		{
			return slotIter->second;
		}
	}

	return DBData();
}

InklingColor VanillaHandler::getInklingColor(Slot slot) const
{
	int iSlot = slot.getInt();

	if (iSlot >= 0 && iSlot < 8)
	{
		return inklingColors[iSlot];
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

bool VanillaHandler::isOkay() const
{
	return okay;
}

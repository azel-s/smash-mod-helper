#pragma once

#include "nlohmann/json.hpp"
#include "ModHandler.h"
#include <string>
#include <filesystem>
#include <fstream>
#include <thread>
#include <codecvt>
#include <queue>
#include <cmath>
namespace fs = std::filesystem;
using std::string, std::ifstream, std::ofstream;
using std::queue;

/* --- HELPERS (UNIVERSAL) --- */
void ModHandler::addFile(string code, string fileType, int slot, string file)
{
	// Convert code to be lowercase.
	for (int i = 0; i < code.length(); i++)
	{
		code[i] = tolower(code[i]);
	}

	if (code == "popo" || code == "nana")
	{
		code = "ice_climber";
	}

	if (!vHandler.getCharName(code).empty())
	{
		replace(file.begin(), file.end(), '\\', '/');

		files[code][fileType][Slot(slot)].insert(Path(file));
		slots[code][Slot(slot)] = Slot(slot);
	}
	else
	{
		wxLog("> Error: " + code + "'s name could not be determined.");
	}
}

void ModHandler::outputUTF(wofstream& file, wxString str, bool parse)
{
	if (parse)
	{
		wxString result = "";

		for (auto i = str.begin(); i != str.end(); i++)
		{
			if (*i == '"')
			{
				result += "&quot;";
			}
			else if (*i == '\'')
			{
				result += "&apos;";
			}
			else if (*i == '&')
			{
				result += "&amp;";
			}
			else if (*i == '<')
			{
				result += "&lt;";
			}
			else if (*i == '>')
			{
				result += "&gt;";
			}
			else if (*i == '|')
			{
				result += '\n';
			}
			else
			{
				result += *i;
			}
		}

		file << result.ToStdWstring();
	}
	else
	{
		file << str.ToStdWstring();
	}
}

void ModHandler::remove_desktop_ini()
{
	int count = 0;

	queue<fs::directory_entry> folders;
	folders.push(fs::directory_entry(path));

	while (!folders.empty())
	{
		for (const auto& i : fs::directory_iterator(folders.front()))
		{
			if (i.is_directory())
			{
				folders.push(i);
			}
			else
			{
				string filename = i.path().filename().string();

				for (int i = 0; i < filename.size(); i++)
				{
					filename[i] = tolower(filename[i]);
				}

				if (filename == "desktop.ini")
				{
					fs::remove(i);
					count++;

					string path = i.path().string();
					replace(path.begin(), path.end(), '\\', '/');
					path = path.substr(path.size());

					wxLog("> Success: Deleted " + path);
				}
			}
		}

		folders.pop();
	}

	if (count == 0)
	{
		wxLog("> Success: No desktop.ini files were found.");
	}
	else
	{
		wxLog("> Success: " + to_string(count) + " desktop.ini files were deleted.");
	}
}

/* --- HELPERS (WX) --- */
void ModHandler::wxLog(string message, bool debug)
{
	if (log && debug)
	{
		log->LogText(message);
	}
}

/* --- TEST FUNCTIONS (WIP/DEBUG) --- */
void ModHandler::test()
{
	string code = "pickel";
	auto db = vHandler.getXMLData(code, Slot(6));

	wxLog("> " + code + ": (" + to_string(db.cIndex) + " " + to_string(db.cGroup) + " " + to_string(db.nIndex) + ")");
	wxLog("> " + code + ": " + db.label);
	wxLog("> " + code + ": " + db.article);
}

/* --- CONSTRUCTORS (UNIVERSAL) --- */
ModHandler::ModHandler(wxLogTextCtrl* log) : log(log)
{
	debug = false;

	fileTypes.push_back("effect");
	fileTypes.push_back("fighter");
	fileTypes.push_back("sound");
	fileTypes.push_back("ui");
}

/* --- SETTERS (UNIVERSAL) --- */
void ModHandler::setSlots(map<string, map<Slot, set<Slot>>> slots)
{
	for (auto i = slots.begin(); i != slots.end(); i++)
	{
		for (auto j = i->second.begin(); j != i->second.end(); j++)
		{
			for (auto k = j->second.begin(); k != j->second.end(); k++)
			{
				this->slots[i->first][*k] = j->first;
			}
		}
	}
}

void ModHandler::setDebug(bool debug)
{
	this->debug = debug;
}

/* --- SETTERS (WX) --- */
void ModHandler::wxSetLog(wxLogTextCtrl* log)
{
	this->log = log;
}

string ModHandler::getPath()
{
	return path;
}

/* --- GETTERS (UNIVERSAL) --- */
string ModHandler::getName(string code)
{
	return vHandler.getCharName(code);
}

string ModHandler::getCode(string name)
{
	return vHandler.getCharCode(name);
}

int ModHandler::getNumCharacters()
{
	return files.size();
}

set<Slot> ModHandler::getAddSlots(string code) const
{
	set<Slot> addSlots;

	// Check if character's file(s) exist(s).
	auto charIter = slots.find(code);
	if (charIter != slots.end())
	{
		// Go through all slots
		for (auto i = charIter->second.begin(); i != charIter->second.end(); i++)
		{
			if (i->first.getInt() > 7 && i->first.getInt() != 999)
			{
				// Ignore kirby slots that only contain copy files.
				if (charIter->first == "kirby")
				{
					// Check if kirby exists in files map (safety measure)
					auto kirbyIter = files.find("kirby");
					if (kirbyIter != files.end())
					{
						// Check if only fighter file type exists
						auto fighterIter = kirbyIter->second.find("fighter");
						if (kirbyIter->second.size() == 1 && fighterIter != kirbyIter->second.end())
						{
							// Check if slot exists (safety measure)
							auto slotIter = fighterIter->second.find(i->first);
							if (slotIter != fighterIter->second.begin())
							{
								// Go through files to check if a non-copy file exists.
								bool copy = true;
								for (auto j = slotIter->second.begin(); j != slotIter->second.end(); j++)
								{
									if (j->getPath().find("copy_") == string::npos)
									{
										copy = false;
										break;
									}
								}

								// Only copy files exist, thus a non-additional slot.
								if (copy)
								{
									continue;
								}
							}
						}
					}
				}

				addSlots.insert(i->first);
			}
		}
	}

	return addSlots;
}

map<string, set<Slot>> ModHandler::getAllSlots() const
{
	map<string, set<Slot>> slots;

	for (auto i = this->slots.begin(); i != this->slots.end(); i++)
	{
		for (auto j = i->second.begin(); j != i->second.end(); j++)
		{
			slots[i->first].insert(j->first);
		}
	}

	return slots;
}

map<string, set<Slot>> ModHandler::getAddSlots() const
{
	map<string, set<Slot>> addSlots;
	for (auto i = slots.begin(); i != slots.end(); i++)
	{
		addSlots[i->first] = getAddSlots(i->first);
	}
	return addSlots;
}

Slot ModHandler::getBaseSlot(string code, Slot slot) const
{
	auto cIter = slots.find(code);
	if (cIter != slots.end())
	{
		auto sIter = cIter->second.find(slot);
		if (sIter != cIter->second.end())
		{
			return sIter->second;
		}
	}
	return Slot(-1);
}

/* --- GETTERS (WX) --- */
wxArrayString ModHandler::wxGetCharacterNames(string fileType) const
{
	wxArrayString characters;
	for (auto i = files.begin(); i != files.end(); i++)
	{
		if (fileType.empty() || i->second.find(fileType) != i->second.end())
		{
			characters.Add(vHandler.getCharName(i->first));
		}
	}
	return characters;
}

wxArrayString ModHandler::wxGetFileTypes(string code) const
{
	wxArrayString fileTypes;
	if (code.empty())
	{
		for (auto& fileType : this->fileTypes)
		{
			fileTypes.Add(fileType);
		}
	}
	else
	{
		auto charIter = files.find(code);
		if (charIter != files.end())
		{
			for (auto i = charIter->second.begin(); i != charIter->second.end(); i++)
			{
				fileTypes.Add(i->first);
			}
		}
	}
	return fileTypes;
}

wxArrayString ModHandler::wxGetFileTypes(wxArrayString codes, bool findInAll) const
{
	set<string> fileTypesSet;

	if (findInAll)
	{
		wxArrayString fileTypes;
		if (!codes.empty())
		{
			fileTypes = wxGetFileTypes(codes[0].ToStdString());

			for (int i = 0; i < fileTypes.size(); i++)
			{
				bool foundInAll = true;

				for (int j = 1; j < codes.size(); j++)
				{
					auto charIter = files.find(codes[j].ToStdString());
					if (charIter != files.end())
					{
						auto fileTypeIter = charIter->second.find(fileTypes[i].ToStdString());
						if (fileTypeIter == charIter->second.end())
						{
							foundInAll = false;
							break;
						}
					}
					else
					{
						foundInAll = false;
						break;
					}
				}

				if (foundInAll)
				{
					fileTypesSet.insert(fileTypes[i].ToStdString());
				}
			}
		}
	}
	else
	{
		for (auto& code : codes)
		{
			auto fileTypes = wxGetFileTypes(code.ToStdString());

			for (auto& fileType : fileTypes)
			{
				fileTypesSet.insert(fileType.ToStdString());
			}
		}
	}

	wxArrayString fileTypes;
	for (auto& fileType : fileTypesSet)
	{
		fileTypes.Add(fileType);
	}
	return fileTypes;
}

wxArrayString ModHandler::wxGetSlots(string code, wxArrayString fileTypes, bool findInAll) const
{
	// First put slots in set so duplicates are accounted for.
	set<Slot> slotsSet;

	auto charIter = files.find(code);
	bool ice_climbers = false;

	if (code == "popo" || code == "nana")
	{
		charIter = files.find("ice_climber");
		ice_climbers = true;
	}

	// Character has files.
	if (charIter != files.end())
	{
		// INFO: When finding in all, make a loop for one file type
		//       so a secondary loop can go through the rest.
		wxArrayString* fileTypesPtr = findInAll ? new wxArrayString : &fileTypes;
		if (findInAll)
		{
			if (fileTypes.empty())
			{
				fileTypes = this->wxGetFileTypes();
			}
			
			// Priorotize non-effect fileType
			if (fileTypes.size() > 1 && fileTypes[0] == "effect")
			{
				fileTypesPtr->Add(fileTypes[1]);
			}
			else
			{
				fileTypesPtr->Add(fileTypes[0]);
			}
		}
		else if (fileTypes.empty())
		{
			// An empty list means to search through all file types.
			fileTypes = this->wxGetFileTypes();
		}

		for (auto& fileType : *fileTypesPtr)
		{
			// Character has file Type
			auto fileTypeIter = charIter->second.find(fileType.ToStdString());
			if (fileTypeIter != charIter->second.end())
			{
				// Go through all slots
				for (auto i = fileTypeIter->second.begin(); i != fileTypeIter->second.end(); i++)
				{
					bool hasSlot = true;

					// INFO: Ice Climber's require a check to see if the slot belongs to popo/nana.
					if (ice_climbers)
					{
						bool slash = fileType == "effect" || fileType == "fighter";
						string label = slash ? "/" + code + "/" : "_" + code + "_";

						// Go through slot's files to check if popo/nana is mentioned
						// and mark hasSlot to true if so.
						hasSlot = false;
						for (auto j = i->second.begin(); j != i->second.end(); j++)
						{
							if (j->getPath().rfind(label) != string::npos)
							{
								hasSlot = true;
								break;
							}
						}
					}

					if (hasSlot)
					{
						if (!findInAll)
						{
							slotsSet.insert(i->first);
						}
						else
						{
							bool slotInAll = true;

							// Verify exsistence in each file type
							for (auto j = 0; j < fileTypes.size(); j++)
							{
								// Character has file Type
								auto fileTypeJter = charIter->second.find(fileTypes[j].ToStdString());
								if (fileTypeJter != charIter->second.end())
								{
									if (fileTypeJter->second.find(i->first) == fileTypeJter->second.end())
									{
										if (fileTypeJter->first != "effect" || fileTypeJter->second.find(Slot(999)) == fileTypeJter->second.end())
										{
											slotInAll = false;
										}
									}
								}
							}

							if (slotInAll)
							{
								slotsSet.insert(i->first);
							}
						}
					}
				}
			}
		}

		if (findInAll)
		{
			delete fileTypesPtr;
		}
	}

	// Put slots in wxArray
	wxArrayString slotsArray;
	for (auto& slot : slotsSet)
	{
		slotsArray.Add((slot.getInt() != 999 ? "c" : "") + slot.getString());
	}
	return slotsArray;
}

wxArrayString ModHandler::wxGetSlots(wxArrayString codes, wxArrayString fileTypes, bool findInAll) const
{
	set<string> slotsSet;

	if (findInAll)
	{
		wxArrayString slots;
		if (!codes.empty())
		{
			slots = wxGetSlots(codes[0].ToStdString(), fileTypes, findInAll);

			for (int i = 0; i < slots.size(); i++)
			{
				bool foundInAll = true;

				for (int j = 1; j < codes.size(); j++)
				{
					auto charIter = this->slots.find(codes[j].ToStdString());
					if (charIter != this->slots.end())
					{
						auto fileTypeIter = charIter->second.find(slots[i].ToStdString());
						if (fileTypeIter == charIter->second.end())
						{
							foundInAll = false;
							break;
						}
					}
					else
					{
						foundInAll = false;
						break;
					}
				}

				if (foundInAll)
				{
					slotsSet.insert(slots[i].ToStdString());
				}
			}
		}
	}
	else
	{
		for (auto& code : codes)
		{
			auto slots = wxGetSlots(code.ToStdString());

			for (auto& slot : slots)
			{
				slotsSet.insert(slot.ToStdString());
			}
		}
	}

	wxArrayString slots;
	for (auto& slot : slotsSet)
	{
		slots.Add(slot);
	}
	return slots;
}

/* --- VERIFIERS (UNIVERSAL) ---*/
bool ModHandler::hasChar(string code) const
{
	if (code.empty())
	{
		return !files.empty();
	}
	else
	{
		return files.find(code) != files.end();
	}
}

bool ModHandler::hasFileType(string fileType) const
{
	for (auto i = files.begin(); i != files.end(); i++)
	{
		if (fileType.empty() && !i->second.empty())
		{
			return true;
		}
		else if (i->second.find(fileType) != i->second.end())
		{
			return true;
		}
	}

	return false;
}

bool ModHandler::hasAddSlot(string code) const
{
	if (code.empty())
	{
		for (auto i = files.begin(); i != files.end(); i++)
		{
			if (hasAddSlot(i->first))
			{
				return true;
			}
		}
	}
	else
	{
		auto cIter = files.find(code);
		if (cIter != files.end())
		{
			// Navigate file types
			for (auto i = cIter->second.begin(); i != cIter->second.end(); i++)
			{
				// Navigate slots
				for (auto j = i->second.begin(); j != i->second.end(); j++)
				{
					if (j->first.getInt() > 7)
					{
						return true;
					}
				}
			}
		}
	}

	return false;
}

/* --- VERIFIERS (WX) --- */
bool ModHandler::wxHasSlot(string code, Slot slot, wxArrayString fileTypes, bool findInAll) const
{
	auto charIter = slots.find(code);
	bool ice_climbers = false;

	if (code == "popo" || code == "nana")
	{
		charIter = slots.find("ice_climber");
		ice_climbers = true;
	}

	if (charIter != slots.end())
	{
		auto slotIter = charIter->second.find(slot);
		if (slotIter != charIter->second.end())
		{
			if (findInAll)
			{
				bool slotInAll = true;

				auto charJter = files.find(ice_climbers ? "ice_climber" : code);
				if (charJter != files.end())
				{
					for (auto& fileType : fileTypes)
					{
						auto fileTypeIter = charJter->second.find(fileType.ToStdString());
						if (fileTypeIter != charJter->second.end())
						{
							auto slotJter = fileTypeIter->second.find(slot);

							if (slotJter == fileTypeIter->second.end())
							{
								slotInAll = false;
							}
							else if (charJter->first == "ice_climber")
							{
								bool slash = fileType == "effect" || fileType == "fighter";
								string label = slash ? "/" + code + "/" : "_" + code + "_";

								bool hasSlot = false;
								for (auto i = slotJter->second.begin(); i != slotJter->second.end(); i++)
								{
									if (i->getPath().rfind(label) != string::npos)
									{
										hasSlot = true;
										break;
									}
								}

								slotInAll = hasSlot;
							}

							if (!slotInAll)
							{
								return false;
							}
						}
					}
				}
				else
				{
					slotInAll = false;
				}

				return slotInAll;
			}
			else
			{
				wxArrayString temp;
				for (auto& fileType : fileTypes)
				{
					temp.Add(fileType);
					if (wxHasSlot(code, slot, temp, true))
					{
						return true;
					}
					temp.RemoveAt(0);
				}

				return false;
			}
		}
	}

	return false;
}

bool ModHandler::wxHasSlot(wxArrayString codes, Slot slot, wxArrayString fileTypes, bool findInAll) const
{
	for (int i = 0; i < codes.size(); i++)
	{
		bool slotFound = wxHasSlot(codes[i].ToStdString(), slot, fileTypes, findInAll);
		if (findInAll && !slotFound)
		{
			return false;
		}
		else if (!findInAll && slotFound)
		{
			return true;
		}
	}

	return findInAll;
}

/* --- FUNCTIONS (UNIVERSAL) --- */
// TODO: Add in slot verification (look at Path Object for reference).
void ModHandler::readFiles(string path)
{
	replace(path.begin(), path.end(), '\\', '/');
	this->path = path;

	for (const auto& i : fs::directory_iterator(path))
	{
		string fileType = i.path().filename().string();

		if (fileType == "effect")
		{
			for (const auto& j : fs::directory_iterator(i))
			{
				if (j.path().filename() == "fighter")
				{
					// Folder = [code] (i.e. ike, koopa, etc.)
					for (const auto& k : fs::directory_iterator(j))
					{
						string code = k.path().filename().string();

						// Effect Files/Folders
						if (k.is_directory())
						{
							for (const auto& l : fs::directory_iterator(k))
							{
								string file = l.path().filename().string();
								int slot;

								// effectFile is an eff file
								if (file.rfind(".eff") != std::string::npos)
								{
									// One-Slotted eff file
									// ef_[code]_c[slot].eff
									// ef_ + _c + .eff = 9
									if (file.size() > (9 + code.size()))
									{
										// ef_[code]_c[slot].eff
										// ef_ + _c = 5
										slot = stoi(file.substr(5 + code.size(), file.find(".eff") - (5 + code.size())));
									}
									// effectFile is not One-Slotted
									else
									{
										slot = 999;
									}

								}
								// effectFile is a folder
								else
								{
									// model folder requires special treatment
									if (file == "model")
									{
										for (const auto& m : fs::directory_iterator(l))
										{
											string effectFile = m.path().filename().string();
											auto cPos = effectFile.rfind("_c");

											// Folder is one slotted
											if (cPos != string::npos && cPos != effectFile.find("_c"))
											{
												slot = stoi(effectFile.substr(cPos + 2));
											}
											else
											{
												slot = 999;
											}

											addFile(code, fileType, slot, m.path().string());
										}

										continue;
									}
									// Character specific folder, likely a Trail folder (One-Slotted)
									else if (file.find("_c") != string::npos)
									{
										// [character_specific_effect]_c[slot]
										// _c = 2
										slot = stoi(file.substr(file.rfind("_c") + 2));
									}
									// Character specific folder, likely a Trail folder (NOT One-Slotted)
									else
									{
										slot = 999;
									}
								}

								// TODO: CHECK
								file = l.path().string();

								addFile(code, fileType, slot, file);
							}
						}
					}
				}
			}
		}
		else if (fileType == "fighter")
		{
			// Folder = [code] (i.e. ike, koopa, etc.)
			for (const auto& j : fs::directory_iterator(i))
			{
				string code = j.path().filename().string();

				// Folder = [folder type] (i.e. model, motion etc.)
				if (j.is_directory())
				{
					for (const auto& k : fs::directory_iterator(j))
					{
						// Folder = [character_specfic_folder] (i.e. body, sword, etc.)
						if (k.is_directory())
						{
							for (const auto& l : fs::directory_iterator(k))
							{
								// Folder = c[slot]
								if (l.is_directory())
								{
									for (const auto& m : fs::directory_iterator(l))
									{
										if (m.is_directory())
										{
											// c[slot]
											// c = 1
											int slot = stoi(m.path().filename().string().substr(1));
											string file = m.path().string();

											addFile(code, fileType, slot, file);
										}
									}
								}
							}
						}
					}
				}
			}
		}
		else if (fileType == "sound")
		{
			for (const auto& j : fs::directory_iterator(i))
			{
				if (j.path().filename() == "bank")
				{
					for (const auto& k : fs::directory_iterator(j))
					{
						// Folder = fighter || fighter_voice
						if (k.is_directory() && (k.path().filename() == "fighter" || k.path().filename() == "fighter_voice"))
						{
							for (const auto& l : fs::directory_iterator(k))
							{
								string filename = l.path().filename().string();
								string file = l.path().string();

								if (filename.find("vc_") == 0 || filename.find("se_") == 0)
								{
									// vc_[code] || se_[code]
									// vc_ = 3       || se_ = 3
									size_t dotPos = filename.rfind('.');

									// Slotted
									if (dotPos != string::npos)
									{
										if (isdigit(filename[dotPos - 1]))
										{
											string code = filename.substr(3, filename.substr(3).find("_"));

											// [...]_c[slot].[filetype]
											// _c = 2
											size_t cPos = filename.rfind("_c");

											if (cPos != string::npos)
											{
												int slot = stoi(filename.substr(cPos + 2, filename.rfind(".") - cPos - 2));
												addFile(code, fileType, slot, file);
											}
											else
											{
												wxLog("> " + filename + "'s slot could not be determined!");
											}
										}
										else
										{
											string code = filename.substr(3, dotPos - 3);
											addFile(code, fileType, -1, file);
										}
									}
									else
									{
										wxLog("> " + filename + "is not a valid sound file!");
									}
								}
							}
						}
					}
				}
			}
		}
		else if (fileType == "ui")
		{
			for (const auto& j : fs::directory_iterator(i))
			{
				if (j.path().filename() == "replace" || j.path().filename() == "replace_patch")
				{
					for (const auto& k : fs::directory_iterator(j))
					{
						if (k.path().filename() == "chara")
						{
							string charaPath = k.path().string();

							for (const auto& l : fs::directory_iterator(k))
							{
								// Folder = chara_[x]
								if (l.is_directory())
								{
									// File = chara_[x]_[code]_[slot].bntx
									for (const auto& m : fs::directory_iterator(l))
									{
										string code = m.path().filename().string().substr(m.path().filename().string().find('_', 6) + 1);

										int underscoreIndex = code.rfind('_');

										if (underscoreIndex != string::npos)
										{
											int slot = stoi(code.substr(underscoreIndex + 1, 2));
											code = code.substr(0, underscoreIndex);

											if (code == "eflame_only" || code == "eflame_first")
											{
												code = "eflame";
											}
											else if (code == "elight_only" || code == "elight_first")
											{
												code = "elight";
											}

											string file = m.path().string();
											addFile(code, fileType, slot, file);
										}
									}
								}
							}
						}
					}
				}
			}
		}
		else
		{
			// Other fileTypes are not currently supported
			// wxLog("> " + fileType + " is currently not supported, no changes will be made to " + fileType);
			continue;
		}
	}
}

/* --- FUNCTIONS (WX) --- */
void ModHandler::adjustFiles(string action, string code, wxArrayString fileTypes, Slot iSlot, Slot fSlot)
{
	if (action == "move" || action == "duplicate" || action == "delete")
	{
		auto charIter = files.find(code);
		if (charIter != files.end())
		{
			for (int i = 0; i < fileTypes.size(); i++)
			{
				auto fileTypeIter = charIter->second.find(fileTypes[i].ToStdString());
				if (fileTypeIter != charIter->second.end())
				{
					auto slotIter = fileTypeIter->second.find(iSlot);
					if (slotIter == fileTypeIter->second.end() && fileTypes[i] == "effect")
					{
						auto slotIter = fileTypeIter->second.find(Slot(999));
					}

					if (slotIter != fileTypeIter->second.end())
					{
						for (auto j = slotIter->second.begin(); j != slotIter->second.end(); j++)
						{
							Path fPath = Path(j->getPath().substr(path.size() + 1));
							if (fPath.getSlot().getInt() != -1)
							{
								fPath.setSlot(fSlot);

								filesystem::path initPath(j->getPath());
								filesystem::path finalPath(path + "/" + fPath.getPath());

								try
								{
									if (action == "move")
									{
										filesystem::rename(initPath, finalPath);
									}
									else if (action == "duplicate")
									{
										filesystem::copy(initPath, finalPath, fs::copy_options::recursive);
									}
									else if (action == "delete")
									{
										filesystem::remove_all(initPath);
									}

									if (action != "delete")
									{
										string sysPath = path + "/" + fPath.getPath();
										addFile(code, fileTypes[i].ToStdString(), fSlot.getInt(), sysPath);
									}
								}
								catch (...)
								{
									if (action == "move")
									{
										wxLog("> Error! " + initPath.string() + " could not be renamed!");
									}
									else if (action == "duplicate")
									{
										wxLog("> Error! " + initPath.string() + " could not be copied!");
									}
									else if (action == "delete")
									{
										wxLog("Error! " + initPath.string() + " could not be deleted!");
									}

									return;
								}
							}
							else
							{
								wxLog("> Error! " + fPath.getPath() + " has an invalid slot!");
							}
						}
					}

					if (action == "move")
					{
						files[code][fileTypes[i].ToStdString()].extract(slotIter->first);
					}
					else if (action == "delete")
					{
						files[code][fileTypes[i].ToStdString()].extract(slotIter->first);

						if (files[code][fileTypes[i].ToStdString()].empty())
						{
							files[code].extract(fileTypes[i].ToStdString());

							if (fileTypes[i] == "fighter")
							{
								std::filesystem::remove_all((path + "/fighter/" + code));
							}

							// Delete empty folders
							// TODO: Check folder before deletion
							/*
							if (!hasFileType(fileTypes[i]))
							{
								std::filesystem::remove_all((this->path + "/" + fileTypes[i]).ToStdString());
							}
							*/
						}

						if (files[code].empty())
						{
							files.extract(code);
						}
					}
				}
				else
				{
					continue;
				}
			}
		}
		else
		{
			wxLog("> Error! " + code + " does not exist!");
		}

		if (iSlot.getInt() == 999)
		{
			wxLog("> Success! " + getName(code) + "'s default slot was moved to c" + fSlot.getString() + "!");
		}
		else
		{
			wxLog("> Success! " + getName(code) + "'s c" + iSlot.getString() + " was moved to c" + fSlot.getString() + "!");
		}
	}
}

void ModHandler::adjustFiles(string action, wxArrayString codes, wxArrayString fileTypes, Slot iSlot, Slot fSlot)
{
	for (auto& code : codes)
	{
		adjustFiles(action, code.ToStdString(), fileTypes, iSlot, fSlot);
	}
}

/* --- FUNCTIONS (CONFIG/PRC) --- */
void ModHandler::getNewDirSlots
(
	// files
	vector<string>& newDirInfos,
	// files
	vector<string>& newDirInfosBase,
	// code, slot, base-file, files
	map<string, map<Slot, map<string, set<string>>>>& shareToVanilla,
	// code, slot, base-file, files
	map<string, map<Slot, map<string, set<string>>>>& shareToAdded,
	// code, slot, section-label, files
	map<string, map<Slot, map<string, set<string>>>>& newDirFiles
)
{
	string charcode = "";
	bool hasElement = hasChar("element");
	bool hasKirby = hasChar("kirby");

	map<Slot, set<Path>> kirbyFiles;
	if (hasKirby)
	{
		auto kirbIter = files.find("kirby");
		auto fightIter = kirbIter->second.find("fighter");

		if (fightIter != kirbIter->second.end())
		{
			for (auto k = fightIter->second.begin(); k != fightIter->second.end(); k++)
			{
				for (auto l = k->second.begin(); l != k->second.end(); l++)
				{
					if (l->getPath().find("/copy_") != string::npos)
					{
						for (const auto& m : fs::directory_iterator(l->getPath()))
						{
							kirbyFiles[k->first].insert(Path(m.path().string().substr(path.size() + 1)));
						}
					}
				}
			}
		}
	}

	auto i = files.begin();
	while (i != files.end())
	{
		if (i->first == "ice_climber")
		{
			if (charcode != "popo" && charcode != "nana")
			{
				charcode = "popo";
			}
			else
			{
				charcode = "nana";
			}
		}
		else if (hasElement || (charcode != "element" || i->first != "eflame"))
		{
			if (charcode == "koopag")
			{
				continue;
			}

			charcode = i->first;
		}

		// Get all slots for charcode
		auto charSlots = slots[i->first];

		bool charHasFighter = i->second.find("fighter") != i->second.end();
		bool charHasEffect = i->second.find("effect") != i->second.end();
		bool charHasSound = i->second.find("sound") != i->second.end();

		if (!hasElement && charcode == "element" && i->first == "eflame")
		{
			charSlots = slots[i->first];
			charHasFighter = false;
			charHasEffect = false;
			charHasSound = false;
		}

		for (auto j = charSlots.begin(); j != charSlots.end(); j++)
		{
			// Skip all slot
			if (j->first.getInt() == 999)
			{
				continue;
			}
			else if (j->first.getInt() == -1)
			{
				wxLog("> Error: -1 slot found, investigation needed!");
			}

			bool slotHasFighter = charHasFighter && i->second["fighter"].find(j->first) != i->second["fighter"].end();
			bool slotHasEffect = charHasEffect && i->second["effect"].find(j->first) != i->second["effect"].end();
			bool slotHasSound = charHasSound && i->second["sound"].find(j->first) != i->second["sound"].end();

			bool additionalSlot = j->first.getInt() > 7;

			// If kirby has only copy abilities in it's additional slot, then mark him as not additional.
			if (charcode == "kirby" && slotHasFighter)
			{
				bool copyOnly = true;
				for (auto k = files[i->first]["fighter"][j->first].begin(); k != files[i->first]["fighter"][j->first].end(); k++)
				{
					if (k->getPath().find("copy_") == string::npos)
					{
						copyOnly = false;
					}
				}

				if (copyOnly)
				{
					additionalSlot = false;
				}
			}

			map<string, set<Path>> jFiles;	// Base files
			set<Path> allFiles;	// Mod files from all filetypes
			set<Path> fighterFiles;
			set<Path> soundFiles;
			set<Path> effectFiles;

			// Read fighter files
			if (slotHasFighter)
			{
				for (auto k = i->second["fighter"][j->first].begin(); k != i->second["fighter"][j->first].end(); k++)
				{
					// Skip popo or nana's file reading if their name is not within the path
					if (i->first == "ice_climber" && k->getPath().find("fighter/" + charcode + "/") == string::npos)
					{
						continue;
					}

					for (const auto& l : fs::directory_iterator(k->getPath()))
					{
						string path = l.path().string().substr(this->path.size() + 1);

						allFiles.insert(path);
						fighterFiles.insert(path);
					}
				}

				if (i->first == "ice_climber" && fighterFiles.empty())
				{
					slotHasFighter = false;
				}
			}

			// Read sound files
			if (slotHasSound)
			{
				for (auto k = i->second["sound"][j->first].begin(); k != i->second["sound"][j->first].end(); k++)
				{
					// Skip popo or nana's file reading if their name is not within the path
					if (i->first == "ice_climber" && k->getPath().find("_" + charcode + ".") == string::npos)
					{
						continue;
					}

					for (const auto& l : fs::directory_iterator(k->getPath()))
					{
						string path = l.path().string().substr(this->path.size() + 1);

						allFiles.insert(path);
						soundFiles.insert(path);
					}
				}

				if (i->first == "ice_climber" && soundFiles.empty())
				{
					slotHasSound = false;
				}
			}

			// Read effect files
			if (slotHasEffect)
			{
				for (auto k = i->second["effect"][j->first].begin(); k != i->second["effect"][j->first].end(); k++)
				{
					// Skip popo or nana's file reading if their name is not within the path
					if (i->first == "ice_climber" && k->getPath().find("effect/fighter/" + charcode + "/") == string::npos)
					{
						continue;
					}

					// [...].eff
					// .eff = 4
					if (k->getPath().substr(k->getPath().size() - 4, 4) == ".eff")
					{
						effectFiles.insert(Path(k->getPath().substr(path.size() + 1)));
					}
					else
					{
						for (const auto& l : fs::directory_iterator(k->getPath()))
						{
							string path = l.path().string().substr(this->path.size() + 1);

							allFiles.insert(Path(path));
							effectFiles.insert(Path(path));
						}
					}
				}

				if (i->first == "ice_climber" && effectFiles.empty())
				{
					slotHasEffect = false;
				}
			}

			// Slot is 7+, add necessary files to:
			// newDirInfos, newDirInfosBase, shareToVanilla, shareToAdded, and newDirFiles
			if (additionalSlot)
			{
				if (vHandler.getFiles(charcode, j->second, jFiles) != 0)
				{
					wxLog("> Error: Unknown error encountered while gathering files from vanilla JSON.");
				}

				// Add newDirInfos and newDirInfosBase
				if (jFiles.find("append") != jFiles.end())
				{
					newDirInfos.push_back("\"fighter/" + charcode + "/append/c" + j->first.getString() + "\"");
				}
				newDirInfos.push_back("\"fighter/" + charcode + "/c" + j->first.getString() + "\"");
				newDirInfos.push_back("\"fighter/" + charcode + "/camera/c" + j->first.getString() + "\"");
				newDirInfosBase.push_back
				(
					"\"fighter/" + charcode + "/c" + j->first.getString() + "/camera\": \"fighter/" +
					charcode + "/c" + j->second.getString() + "/camera\""
				);
				// TODO: Not sure why I added nana and kirby as a check here, possible issue?
				if (charcode != "nana" && charcode != "kirby" && jFiles.find("kirbycopy") != jFiles.end())
				{
					newDirInfos.push_back("\"fighter/" + charcode + "/kirbycopy/c" + j->first.getString() + "\"");
					newDirInfosBase.push_back
					(
						"\"fighter/" + charcode + "/kirbycopy/c" + j->first.getString() + "/bodymotion\": \"fighter/"
						+ charcode + "/kirbycopy/c" + j->second.getString() + "/bodymotion\""
					);
					newDirInfosBase.push_back
					(
						"\"fighter/" + charcode + "/kirbycopy/c" + j->first.getString() + "/cmn\": \"fighter/" +
						charcode + "/kirbycopy/c" + j->second.getString() + "/cmn\""
					);
					newDirInfosBase.push_back
					(
						"\"fighter/" + charcode + "/kirbycopy/c" + j->first.getString() + "/sound\": \"fighter/" +
						charcode + "/kirbycopy/c" + j->second.getString() + "/sound\""
					);
				}
				newDirInfos.push_back("\"fighter/" + charcode + "/movie/c" + j->first.getString() + "\"");
				newDirInfos.push_back("\"fighter/" + charcode + "/result/c" + j->first.getString() + "\""); newDirInfosBase.push_back
				(
					"\"fighter/" + charcode + "/c" + j->first.getString() + "/cmn\": \"fighter/" +
					charcode + "/c" + j->second.getString() + "/cmn\""
				);

				for (auto k = jFiles.begin(); k != jFiles.end(); k++)
				{
					if (k->first == "effect")
					{
						continue;
					}

					for (auto l = k->second.begin(); l != k->second.end(); l++)
					{
						Path path = *l;

						if (path.getSlot().getInt() != -1)
						{
							path.setSlot(j->first);
							string label;

							// Camera Files
							if (k->first == "camera")
							{
								label = "\"fighter/" + charcode + "/camera/c" + j->first.getString() + "\"";
								shareToAdded[i->first][j->second]["\"" + l->getPath() + "\""].insert("\"" + path.getPath() + "\"");
							}
							// Fighter Files
							else if (k->first == "vanilla" || k->first == "added")
							{
								label = "\"fighter/" + charcode + "/c" + j->first.getString() + "\"";

								if (allFiles.find(path) == allFiles.end())
								{
									if (k->first == "vanilla")
									{
										shareToVanilla[i->first][j->second]["\"" + l->getPath() + "\""].insert("\"" + path.getPath() + "\"");
									}
									else
									{
										shareToAdded[i->first][j->second]["\"" + l->getPath() + "\""].insert("\"" + path.getPath() + "\"");
									}
								}
							}
							// Movie Files
							else if (k->first == "movie")
							{
								label = "\"fighter/" + charcode + "/movie/c" + j->first.getString() + "\"";

								if (allFiles.find(path) == allFiles.end())
								{
									if (path.getPath().find("/motion/") == string::npos)
									{
										shareToVanilla[i->first][j->second]["\"" + l->getPath() + "\""].insert("\"" + path.getPath() + "\"");
									}
									else
									{
										shareToAdded[i->first][j->second]["\"" + l->getPath() + "\""].insert("\"" + path.getPath() + "\"");
									}
								}
							}
							// Result Files
							else if (k->first == "result")
							{
								label = "\"fighter/" + charcode + "/result/c" + j->first.getString() + "\"";

								if (allFiles.find(path) == allFiles.end())
								{
									if (path.getPath().find("/motion/") == string::npos)
									{
										shareToVanilla[i->first][j->second]["\"" + l->getPath() + "\""].insert("\"" + path.getPath() + "\"");
									}
									else
									{
										shareToAdded[i->first][j->second]["\"" + l->getPath() + "\""].insert("\"" + path.getPath() + "\"");
									}
								}
							}
							// Kirby Files
							else if (k->first == "kirbycopy")
							{
								label = "\"fighter/" + charcode + "/kirbycopy/c" + j->first.getString() + "\"";

								if (!hasKirby || kirbyFiles.empty() || kirbyFiles.find(j->first) == kirbyFiles.end() || kirbyFiles[j->first].find(path) == kirbyFiles[j->first].end())
								{
									if (path.getPath().find("/motion/") == string::npos)
									{
										shareToVanilla[i->first][j->second]["\"" + l->getPath() + "\""].insert("\"" + path.getPath() + "\"");
									}
									else
									{
										shareToAdded[i->first][j->second]["\"" + l->getPath() + "\""].insert("\"" + path.getPath() + "\"");
									}
								}
							}
							else if (false) // TODO: FinalSmash)
							{

							}

							newDirFiles[i->first][j->first][label].insert("\"" + path.getPath() + "\"");
						}
						else
						{
							wxLog("> WARN: Ignored " + l->getPath(), true);
						}
					}
				}

				// Add empty sections if non-existent
				auto temp = &newDirFiles[i->first][j->first];

				if (temp->find("\"fighter/" + charcode + "/c" + j->first.getString() + "\"") == temp->end())
				{
					(*temp)["\"fighter/" + charcode + "/c" + j->first.getString() + "\""];
				}
				if (temp->find("\"fighter/" + charcode + "/camera/c" + j->first.getString() + "\"") == temp->end())
				{
					(*temp)["\"fighter/" + charcode + "/camera/c" + j->first.getString() + "\""];
				}
				if (temp->find("\"fighter/" + charcode + "/kirbycopy/c" + j->first.getString() + "\"") == temp->end())
				{
					(*temp)["\"fighter/" + charcode + "/kirbycopy/c" + j->first.getString() + "\""];
				}
				if (temp->find("\"fighter/" + charcode + "/movie/c" + j->first.getString() + "\"") == temp->end())
				{
					(*temp)["\"fighter/" + charcode + "/movie/c" + j->first.getString() + "\""];
				}
				if (temp->find("\"fighter/" + charcode + "/result/c" + j->first.getString() + "\"") == temp->end())
				{
					(*temp)["\"fighter/" + charcode + "/result/c" + j->first.getString() + "\""];
				}

				/* Adds NEW kirby copy files
				// Special Case for kirby
				if (i->first == "kirby" && slotHasFighter)
				{
					for (auto k = vanillaKirbyFiles.begin(); k != vanillaKirbyFiles.end(); k++)
					{
						if (k->second.find(baseSlot) != k->second.end())
						{
							for (auto l = k->second[baseSlot].begin(); l != k->second[baseSlot].end(); l++)
							{
								string path = *l;
								int cPos = path.find("/c" + baseSlot);

								if (cPos != string::npos)
								{
									path.replace(cPos, 2 + baseSlot.size(), "/c" + *j);

									if (kirbyFiles[j->ToStdString()].find(path) == kirbyFiles[j->ToStdString()].end())
									{
										shareToVanilla[i->first][baseSlot]["\"" + *l + "\""].insert("\"" + path + "\"");
									}
								}

								newDirFiles[i->first][j->ToStdString()]["\"fighter/" + k->first + "/kirbycopy/c" + j->ToStdString() + "\""].insert("\"" + path + "\"");
							}
						}
					}

					for (auto k = addedKirbyFiles.begin(); k != addedKirbyFiles.end(); k++)
					{
						if (k->second.find(baseSlot) != k->second.end())
						{
							for (auto l = k->second[baseSlot].begin(); l != k->second[baseSlot].end(); l++)
							{
								string path = *l;
								int cPos = path.find("/c" + baseSlot);

								if (cPos != string::npos)
								{
									path.replace(cPos, 2 + baseSlot.size(), "/c" + *j);

									if (kirbyFiles[j->ToStdString()].find(path) == kirbyFiles[j->ToStdString()].end())
									{
										shareToAdded[i->first][baseSlot]["\"" + *l + "\""].insert("\"" + path + "\"");
									}
								}

								newDirFiles[i->first][j->ToStdString()]["\"fighter/" + k->first + "/kirbycopy/c" + j->ToStdString() + "\""].insert("\"" + path + "\"");
							}
						}
					}
				}
				*/
			}

			// Add NEW fighter files to newDirFiles
			if (slotHasFighter)
			{
				if (!additionalSlot && vHandler.getFiles(i->first, j->first, jFiles) != 0)
				{
					wxLog("> Error: Unknown error encountered while gathering files from vanilla JSON.");
				}

				for (auto k = fighterFiles.begin(); k != fighterFiles.end(); k++)
				{
					if (additionalSlot)
					{
						Path basePath = Path(*k);
						basePath.setSlot(j->second);

						bool found = false;
						for (auto l = jFiles.begin(); l != jFiles.end(); l++)
						{
							if (l->second.find(basePath) != l->second.end())
							{
								found = true;
								break;
							}
						}

						if (!found)
						{
							if (false)
							{
								//TODO: Check if exists in Kirby Vanilla files.)
							}
							else
							{
								newDirFiles[i->first][j->first]["\"fighter/" + charcode + "/c" + j->first.getString() + "\""].insert("\"" + k->getPath() + "\"");
							}
						}
					}
					else
					{
						bool found = false;
						for (auto l = jFiles.begin(); l != jFiles.end(); l++)
						{
							if (l->second.find(*k) != l->second.end())
							{
								found = true;
								break;
							}
						}

						if (!found)
						{
							if (false)
							{
								//TODO: Check if exists in Kirby Vanilla files.)
							}
							else
							{
								if (charcode == "kirby")
								{
									size_t copyPos = k->getPath().find("copy_");
									if (copyPos != string::npos)
									{
										// fighter/kirby/[motion/body]/copy_[charcode]_[...]/c[slot]/...
										string charCopy = k->getPath().substr(copyPos + 5, k->getPath().find("_", copyPos + 5) - copyPos - 5);
										size_t slotPos = k->getPath().find("/c", copyPos);

										if (slotPos != string::npos)
										{
											string charSlot = k->getPath().substr(slotPos + 2, k->getPath().find('/', slotPos + 1) - slotPos - 2);

											if (false) // TODO: kirby check: vanillaKirbyFiles[charCopy][charSlot].find(*k) != vanillaKirbyFiles[charCopy][charSlot].end())
											{
												found = true;
											}
										}
									}

									if (!found)
									{

									}
								}
								else
								{
									newDirFiles[i->first][j->first]["\"fighter/" + charcode + "/c" + j->first.getString() + "\""].insert("\"" + k->getPath() + "\"");
								}
							}
						}
					}
				}
			}

			// Add One-Slot effect files
			if (slotHasEffect)
			{
				auto effIter = jFiles.find("effect");
				if (effIter != jFiles.end())
				{
					// Add MISSING effect files to shareToVanilla & newDirFiles
					for (auto k = effIter->second.begin(); k != effIter->second.end(); k++)
					{
						Path path = *k;
						path.setSlot(j->first);

						if (effectFiles.find(path) == effectFiles.end())
						{
							shareToVanilla[i->first][Slot(999)]["\"" + k->getPath() + "\""].insert("\"" + path.getPath() + "\"");
							newDirFiles[i->first][j->first]["\"fighter/" + charcode + "/c" + j->first.getString() + "\""].insert("\"" + path.getPath() + "\"");
						}
					}

					// Add NEW effect files to newDirFiles
					for (auto k = effectFiles.begin(); k != effectFiles.end(); k++)
					{
						if (effIter->second.find(*k) == effIter->second.end())
						{
							newDirFiles[i->first][j->first]["\"fighter/" + charcode + "/c" + j->first.getString() + "\""].insert("\"" + k->getPath() + "\"");
						}
					}
				}
			}
		}

		if (!hasElement && charcode == "eflame")
		{
			charcode = "element";
		}
		else if (charcode != "popo")
		{
			i++;
		}
	}
}

// Creators
void ModHandler::create_config()
{
	vector<string> newDirInfos;
	vector<string> newDirInfosBase;
	map<string, map<Slot, map<string, set<string>>>> shareToVanilla;
	map<string, map<Slot, map<string, set<string>>>> shareToAdded;
	map<string, map<Slot, map<string, set<string>>>> newDirFiles;

	getNewDirSlots(newDirInfos, newDirInfosBase, shareToVanilla, shareToAdded, newDirFiles);

	if (!newDirInfos.empty() || !newDirInfosBase.empty() || !shareToVanilla.empty() || !shareToAdded.empty() || !newDirFiles.empty())
	{
		// Start writing config file.
		bool first = true;

		ofstream file(path + "/" + "config.json", std::ios::out | std::ios::trunc);
		if (file.is_open())
		{
			file << "{";

			if (this->hasAddSlot())
			{
				file << "\n\t\"new-dir-infos\": [";
				for (auto i = newDirInfos.begin(); i != newDirInfos.end(); i++)
				{
					if (first)
					{
						first = false;
					}
					else
					{
						file << ",";
					}

					file << "\n\t\t" << *i;
				}
				file << "\n\t]";

				if (!first)
				{
					file << ",";
					first = true;
				}

				file << "\n\t\"new-dir-infos-base\": {";
				for (auto i = newDirInfosBase.begin(); i != newDirInfosBase.end(); i++)
				{
					if (first)
					{
						first = false;
					}
					else
					{
						file << ",";
					}

					file << "\n\t\t" << *i;
				}
				file << "\n\t}";
			}

			vector sectionType = { &shareToVanilla, &shareToAdded, &newDirFiles };
			for (auto i = 0; i < sectionType.size(); i++)
			{
				if (!((*sectionType[i]).empty()))
				{
					if (!first)
					{
						file << ",";
						first = true;
					}

					if (i == 0)
					{
						file << "\n\t\"share-to-vanilla\": {";
					}
					else if (i == 1)
					{
						file << "\n\t\"share-to-added\": {";
					}
					else
					{
						file << "\n\t\"new-dir-files\": {";
					}

					for (auto j = sectionType[i]->begin(); j != sectionType[i]->end(); j++)
					{
						for (auto k = j->second.begin(); k != j->second.end(); k++)
						{
							for (auto l = k->second.begin(); l != k->second.end(); l++)
							{
								bool firstBox = true;

								for (auto m = l->second.begin(); m != l->second.end(); m++)
								{
									if (first)
									{
										first = false;
									}
									else
									{
										file << ",";
									}

									if (firstBox)
									{
										file << "\n\t\t" + l->first << ": [";
										firstBox = false;
									}

									file << "\n\t\t\t" << *m;
								}

								if (!firstBox)
								{
									file << "\n\t\t]";
								}
								else
								{
									if (first)
									{
										first = false;
									}
									else
									{
										file << ",";
									}

									file << "\n\t\t" + l->first << ": []";
								}
							}
						}
					}

					file << "\n\t}";
				}
			}
			file << "\n}";

			file.close();
			wxLog("> Success: Config file was created!");
		}
		else
		{
			wxLog("> Error: " + (path + "/" + "config.json") + " could not be opened!");
		}
	}
	else
	{
		wxLog("> WARN: config.json is not needed.");
	}
}

void ModHandler::create_message_xmsbt(map<string, map<Slot, Name>>& names)
{
	ofstream msg;
	if (!names.empty())
	{
		msg.open("msg_name.xmsbt", ios::out | ios::binary);

		if (msg.is_open())
		{
			// UTF-16 LE BOM
			msg << (unsigned char)0xFF << (unsigned char)0xFE;

			msg.close();

			wofstream msgUTF;
			msgUTF.open("msg_name.xmsbt", ios::binary | ios::app);
			msgUTF.imbue(std::locale(msgUTF.getloc(), new std::codecvt_utf16<wchar_t, 0x10ffff, std::little_endian>));

			if (msgUTF.is_open())
			{
				outputUTF(msgUTF, "<?xml version=\"1.0\" encoding=\"utf-16\"?>");
				outputUTF(msgUTF, "\n<xmsbt>");

				for (auto i = names.begin(); i != names.end(); i++)
				{
					for (auto j = i->second.begin(); j != i->second.end(); j++)
					{
						string nIndex = Slot(j->first.getInt() + 8).getString();

						if (j->first.getInt() == 0)
						{
							outputUTF(msgUTF, "\n\t<entry label=\"nam_chr3_" + nIndex + "_" + i->first + "\">");
							outputUTF(msgUTF, "\n\t\t<text>");
							outputUTF(msgUTF, j->second.cssName, true);
							outputUTF(msgUTF, "</text>");
							outputUTF(msgUTF, "\n\t</entry>");
						}

						outputUTF(msgUTF, "\n\t<entry label=\"nam_chr1_" + nIndex + "_" + i->first + "\">");
						outputUTF(msgUTF, "\n\t\t<text>");
						outputUTF(msgUTF, j->second.cspName, true);
						outputUTF(msgUTF, "</text>");
						outputUTF(msgUTF, "\n\t</entry>");

						outputUTF(msgUTF, "\n\t<entry label=\"nam_chr2_" + nIndex + "_" + i->first + "\">");
						outputUTF(msgUTF, "\n\t\t<text>");
						outputUTF(msgUTF, j->second.vsName, true);
						outputUTF(msgUTF, "</text>");
						outputUTF(msgUTF, "\n\t</entry>");

						if (i->first != "eflame_first" && i->first != "elight_first")
						{
							outputUTF(msgUTF, "\n\t<entry label=\"nam_stage_name_" + nIndex + "_" + i->first + "\">");
							outputUTF(msgUTF, "\n\t\t<text>");
							outputUTF(msgUTF, j->second.stageName, true);
							outputUTF(msgUTF, "</text>");
							outputUTF(msgUTF, "\n\t</entry>");
						}
					}
				}

				outputUTF(msgUTF, "\n</xmsbt>");
				msgUTF.close();

				fs::create_directories(path + "/ui/message/");
				fs::rename(fs::current_path() / "msg_name.xmsbt", path + "/ui/message/msg_name.xmsbt");
			}
			else
			{
				wxLog("> " + fs::current_path().string() + "/msg_name.xmsbt could not be opened!");
			}
		}
		else
		{
			wxLog("> Error: " + fs::current_path().string() + "/msg_name.xmsbt could not be opened!");
		}
	}
}

void ModHandler::create_db_prcxml(map<string, map<Slot, Name>>& names, map<string, map<Slot, string>>& announcers, map<string, Slot>& maxSlots)
{
	if (!maxSlots.empty() || !announcers.empty() || !names.empty())
	{
		ifstream uiVanilla("ui_chara_db.xml");
		ofstream uiEdit("ui_chara_db.prcxml");

		if (uiVanilla.is_open() && uiEdit.is_open())
		{
			uiEdit << "<?xml version=\"1.0\" encoding=\"UTF-16\"?>";
			uiEdit << "\n<struct>";
			uiEdit << "\n\t<list hash=\"db_root\">";

			string code;
			string line;
			string currIndex = "";
			char status = 0;

			while (!uiVanilla.eof())
			{
				getline(uiVanilla, line);

				if (line.find("<struct index=") != string::npos)
				{
					currIndex = line.substr(line.find('"') + 1, line.rfind('"') - line.find('"') - 1);
				}
				else if (line.find("\"name_id\"") != string::npos)
				{
					int begin = line.find(">") + 1;
					int end = line.find("<", begin);
					code = line.substr(begin, end - begin);

					// Deal with max-slots first
					auto charIter = maxSlots.find(code);
					if (charIter != maxSlots.end())
					{
						status = 1;
						uiEdit << "\n\t\t<struct index=\"" << currIndex << "\">";
						uiEdit << "\n\t\t\t<byte hash=\"color_num\">" << charIter->second.getString() << "</byte>";

						// Add cIndex/cGroup info
						auto charLter = slots.find(code);
						if (charLter != slots.end())
						{
							for (int i = 8; i < charIter->second.getInt(); i++)
							{
								auto slotIter = charLter->second.find(Slot(i));
								if (slotIter != charLter->second.end())
								{
									auto db = vHandler.getXMLData(code, slotIter->second);

									if (db.cIndex != 0)
									{
										uiEdit << "\n\t\t\t<byte hash=\"c" + Slot(i).getString() + "_index\">" << to_string(db.cIndex) << "</byte>";
									}
									if (db.cGroup != 0)
									{
										uiEdit << "\n\t\t\t<byte hash=\"c" + Slot(i).getString() + "_index\">" << to_string(db.cGroup) << "</byte>";
									}
								}
							}
						}
					}

					// Deal with names second
					auto charJter = names.find(code);
					if (!names.empty() && charJter != names.end())
					{
						if (status != 1)
						{
							uiEdit << "\n\t\t<struct index=\"" << currIndex << "\">";
							status = 1;
						}

						auto i = charJter->second.begin();
						while (i != charJter->second.end())
						{
							string nameSlot = Slot(i->first.getInt() + 8).getString();
							uiEdit << "\n\t\t\t<byte hash=\"n" << i->first.getString() << "_index\">" << nameSlot << "</byte>";
							i++;
						}
					}

					// Deal with announcers third
					auto charKter = announcers.find(code);
					if (!announcers.empty() && charKter != announcers.end())
					{
						if (status != 1)
						{
							uiEdit << "\n\t\t<struct index=\"" << currIndex << "\">";
							status = 1;
						}

						auto charLter = slots.find(code);
						if (charLter != slots.end())
						{
							for (auto i = charKter->second.begin(); i != charKter->second.end(); i++)
							{
								auto slotIter = charLter->second.find(i->first);
								if (slotIter != charLter->second.end())
								{
									auto db = vHandler.getXMLData(code, slotIter->second);
									string slot = Slot(i->first.getInt() + 8).getString();

									if (i->second == "Default")
									{
										uiEdit << "\n\t\t\t<hash40 hash=\"characall_label_article_c" + slot + "\">" + db.label << "</hash40>";
										if (!db.article.empty())
										{
											uiEdit << "\n\t\t\t<hash40 hash=\"characall_label_article_c" + slot + "\">" + db.article << "</hash40>";
										}
									}
									else
									{
										uiEdit << "<hash40 hash=\"characall_label_article_c" + slot + "\">" + i->second << "</hash40>";
									}
								}
							}
						}
					}
				}
				else if (line.find("</struct>") != string::npos)
				{
					if (status == 0)
					{
						uiEdit << "\n\t\t<hash40 index=\"" << currIndex << "\">dummy</hash40>";
					}
					else if (status == 1)
					{
						uiEdit << "\n\t\t</struct>";
						status = 0;
					}
					else
					{
						status = 0;
					}

					// Skip everything else as demon is the last code.
					if (currIndex == "120")
					{
						break;
					}
				}
			}

			uiEdit << "\n\t</list>";
			uiEdit << "\n</struct>";

			uiVanilla.close();
			uiEdit.close();

			fs::remove(fs::current_path() / "ui_chara_db.xml");
		}
		else if (!uiVanilla.is_open())
		{
			wxLog("> Error: " + fs::current_path().string() + "/ui_chara_db.xml could not be opened!");
		}
		else
		{
			wxLog("> Error: " + fs::current_path().string() + "/ui_chara_db_EDIT.xml could not be opened!");
		}
	}
	else
	{
		wxLog("> WARN: ui_chara_db.prcxml is not needed for the requested changes.");
	}
}

void ModHandler::create_ink_prcxml(map<Slot, InklingColor>& inklingColors)
{
	if (!inklingColors.empty())
	{
		ifstream effectVanilla("effect.prcxml");
		ofstream effectEdit("effect_EDIT.prcxml");

		if (effectVanilla.is_open() && effectEdit.is_open())
		{
			string line;
			while (!effectVanilla.eof())
			{
				getline(effectVanilla, line);

				if (line.find("hash=\"ink_") != string::npos)
				{
					char action;

					if (line.find("ink_effect_color") != string::npos)
					{
						action = 'E';
					}
					else
					{
						action = 'A';
					}

					effectEdit << line << endl;

					// Fill out Slots 0-7
					auto iter = inklingColors.begin();
					for (int i = 0; i < 8; i++)
					{
						auto iter = inklingColors.find(Slot(i));
						if (iter != inklingColors.end())
						{
							effectEdit << "    <struct index=\"" << to_string(i) << "\">" << endl;

							if (action == 'E')
							{
								effectEdit << "      <float hash=\"r\">" << (iter->second.effect.Red() / 255.0) << "</float>" << endl;
								effectEdit << "      <float hash=\"g\">" << (iter->second.effect.Green() / 255.0) << "</float>" << endl;
								effectEdit << "      <float hash=\"b\">" << (iter->second.effect.Blue() / 255.0) << "</float>" << endl;
							}
							else
							{
								effectEdit << "      <float hash=\"r\">" << (iter->second.arrow.Red() / 255.0) << "</float>" << endl;
								effectEdit << "      <float hash=\"g\">" << (iter->second.arrow.Green() / 255.0) << "</float>" << endl;
								effectEdit << "      <float hash=\"b\">" << (iter->second.arrow.Blue() / 255.0) << "</float>" << endl;
							}

							effectEdit << "    </struct>" << endl;
						}
						else
						{
							effectEdit << "    <hash40 index=\"" << to_string(i) << "\">dummy</hash40>" << endl;
						}
					}

					// Move iter to earliest additional slot
					while (iter != inklingColors.end() && iter->first.getInt() <= 7)
					{
						iter++;
					}

					while (iter != inklingColors.end())
					{
						// to_string used because index cannot start with 0.
						effectEdit << "    <struct index=\"" << to_string(iter->first.getInt()) << "\">" << endl;

						if (action == 'E')
						{
							effectEdit << "      <float hash=\"r\">" << (iter->second.effect.Red() / 255.0) << "</float>" << endl;
							effectEdit << "      <float hash=\"g\">" << (iter->second.effect.Green() / 255.0) << "</float>" << endl;
							effectEdit << "      <float hash=\"b\">" << (iter->second.effect.Blue() / 255.0) << "</float>" << endl;
						}
						else
						{
							effectEdit << "      <float hash=\"r\">" << (iter->second.arrow.Red() / 255.0) << "</float>" << endl;
							effectEdit << "      <float hash=\"g\">" << (iter->second.arrow.Green() / 255.0) << "</float>" << endl;
							effectEdit << "      <float hash=\"b\">" << (iter->second.arrow.Blue() / 255.0) << "</float>" << endl;
						}

						effectEdit << "    </struct>" << endl;
					}
				}
				else
				{
					effectEdit << line << endl;
				}
			}

			effectVanilla.close();
			effectEdit.close();
		}
		else
		{
			if (!effectVanilla.is_open())
			{
				wxLog("> Error: " + fs::current_path().string() + "/effect.prcxml could not be opened!");
			}

			if (!effectEdit.is_open())
			{
				wxLog("> Error: " + fs::current_path().string() + "/effect_EDIT.prcxml could not be opened!");
			}
		}
	}
}

// Readers
map<string, map<Slot, Slot>> ModHandler::read_config_slots()
{
	map<string, map<Slot, Slot>> slots;
	if (fs::exists(path + "/config.json"))
	{
		ifstream inFile(path + "/config.json");
		if (inFile.is_open())
		{
			// Reach section where base slots can be easily found.
			string line = "";
			while (line.find("\"new-dir-infos-base\"") == string::npos && !inFile.eof())
			{
				getline(inFile, line);
			}

			// Make sure this is not end of file.
			if (line.find("\"new-dir-infos-base\"") != string::npos)
			{
				while (line.find('}') == string::npos && !inFile.eof())
				{
					getline(inFile, line);

					auto camPos = line.find("/camera");
					if (camPos != string::npos)
					{
						auto beg = line.find("fighter/") + 8;
						auto end = line.find("/", beg);
						string code = line.substr(beg, end - beg);

						Slot newSlot = Slot(line.substr(end + 2, camPos - end - 2));

						end = line.find("/camera", camPos + 7) - 1;
						beg = line.find(code + "/c", camPos + 7) + 2 + code.size();

						slots[code][newSlot] = Slot(line.substr(beg, end - beg + 1));
					}
				}
			}
		}
		else
		{
			wxLog("> ERROR: " + path + "/config.json" + "could not be opened!");
		}
	}
	return slots;
}

map<string, map<Slot, Name>> ModHandler::read_message_names()
{
	map<string, map<Slot, Name>> names;
	int count = 0;

	if (fs::exists(path + "/ui/message/msg_name.xmsbt"))
	{
		wifstream inFile(this->path + "/ui/message/msg_name.xmsbt", ios::binary);
		inFile.imbue(std::locale(inFile.getloc(), new std::codecvt_utf16<wchar_t, 0x10ffff, std::consume_header>));

		if (inFile.is_open())
		{
			vector<string> lines;
			int count = -2;

			// Read Header
			for (wchar_t c; inFile.get(c) && count < 0; )
			{
				if ((char(c)) == '>')
				{
					count++;
				}
			}

			lines.push_back("");

			for (wchar_t c; inFile.get(c); )
			{
				lines[count] += (char)c;

				if ((char(c)) == '>')
				{
					lines.push_back("");
					count++;
				}
			}

			char type;
			bool action = false;
			size_t beg;
			size_t end;

			// Interpret Data
			for (int i = 0; i < lines.size(); i++)
			{
				// CSS/CSP/VS
				if (lines[i].find("nam_chr") != string::npos)
				{
					beg = lines[i].find("nam_chr") + 9;
					end = lines[i].find("_", beg);
					type = lines[i][beg - 2];

					action = true;
				}
				// Stage_Name
				else if (lines[i].find("nam_stage") != string::npos)
				{
					beg = lines[i].find("nam_stage") + 15;
					end = lines[i].find("_", beg + 1);
					type = 's';

					action = true;
				}

				if (action)
				{
					Slot slot = Slot(stoi(lines[i].substr(beg, end - beg)) - 8);
					string code = lines[i].substr(end + 1, lines[i].find("\"", end + 1) - end - 1);
					string name = lines[i + 2].substr(0, lines[i + 2].find("<"));

					if (type == '1')
					{
						names[code][slot].cspName = name;
					}
					else if (type == '2')
					{
						names[code][slot].vsName = name;
					}
					else if (type == '3')
					{
						names[code][slot].cssName = name;
					}
					else if (type == 's')
					{
						names[code][slot].stageName = name;
					}
					else
					{
						wxLog("> ERROR: Unable to correctly read names from msg_name.xmsbt!");
					}

					action = false;
					i += 2;
				}
			}

			inFile.close();
		}
		else
		{
			wxLog("> ERROR: " + path + "/ui/message/msg_name.xmsbt" + "could not be opened!");
		}
	}

	return names;
}

map<Slot, InklingColor> ModHandler::read_ink_colors()
{
	map<Slot, InklingColor> inkColors;

	// Read XML
	if (fs::exists(path + "/fighter/common/param/effect.prcxml"))
	{
		ifstream inFile(path + "/fighter/common/param/effect.prcxml");
		char action = 'F';
		string line;

		// Go through effect.prcxml line by line
		while (getline(inFile, line))
		{
			// If Inkling related data is found, make a copy of the data.
			if (line.find("ink_effect_color") != string::npos)
			{
				action = 'E';
			}
			else if (line.find("ink_arrow_color") != string::npos)
			{
				action = 'A';
			}

			if (action != 'F')
			{
				while (line.find("</list>") == string::npos)
				{
					if (line.find("<struct") != string::npos)
					{
						auto start = line.find("\"");
						auto end = line.rfind("\"");

						Slot slot = Slot(line.substr(start + 1, end - start - 1));

						getline(inFile, line);
						double red = stod(line.substr(line.find(">") + 1, line.rfind("<") - line.find(">") - 1));

						getline(inFile, line);
						double green = stod(line.substr(line.find(">") + 1, line.rfind("<") - line.find(">") - 1));

						getline(inFile, line);
						double blue = stod(line.substr(line.find(">") + 1, line.rfind("<") - line.find(">") - 1));

						// Red
						if (action == 'E')
						{
							inkColors[slot].effect.Set(red * 255, green * 255, blue * 255);
						}
						else
						{
							inkColors[slot].arrow.Set(red * 255, green * 255, blue * 255);
						}
					}

					getline(inFile, line);
				}

				action = 'F';
			}
		}
	}

	return inkColors;
}

/* --- RESET/CLEANUP --- */
void ModHandler::clear()
{
	for (auto i = files.begin(); i != files.end(); i++)
	{
		for (auto j = i->second.begin(); j != i->second.end(); j++)
		{
			for (auto k = j->second.begin(); k != j->second.end(); k++)
			{
				k->second.clear();
			}
			j->second.clear();
		}
		i->second.clear();
	}

	for (auto i = slots.begin(); i != slots.end(); i++)
	{
		i->second.clear();
	}

	files.clear();
	slots.clear();
}
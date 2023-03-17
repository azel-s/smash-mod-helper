#include "SmashData.h"
#include <string>
#include <filesystem>
#include <fstream>
#include <thread>
#include <codecvt>
namespace fs = std::filesystem;
using std::string, std::ifstream, std::ofstream;

wxString getSlotFromInt(int slot)
{
	if (slot > 9)
	{
		return to_string(slot);
	}
	else if (slot >= 0)
	{
		return ("0" + to_string(slot));
	}
	else
	{
		return "-1";
	}
}

// Constructor
SmashData::SmashData()
{
	fileTypes.push_back("effect");
	fileTypes.push_back("fighter");
	fileTypes.push_back("sound");
	fileTypes.push_back("ui");

	vanillaThreadActive = false;
	stopVanillaThread = false;
}

// Convert paths
string SmashData::convertPath(string input)
{
	std::replace(input.begin(), input.end(), '\\', '/');

	if (input.find(rootPath) != string::npos)
	{
		return input.substr(rootPath.size() + 1);
	}
	else
	{
		return rootPath + '/' + input;
	}
}

// Read Data
void SmashData::readVanillaFiles(string fileType)
{
	for (int i = 0; i < fileTypes.size(); i++)
	{
		ifstream file("files/" + fileType + ".data");

		if (file.is_open())
		{
			string filePath;
			string charcode;

			while (!file.eof())
			{
				// Stop (close was likely pressed)
				if (stopVanillaThread)
				{
					file.close();
					return;
				}

				getline(file, filePath);

				if (fileType == "names")
				{
					auto commaPos = filePath.find(',');
					string name = filePath.substr(commaPos + 1);
					charcode = filePath.substr(0, commaPos);

					charNames[charcode] = name;
					charCodes[name] = charcode;
				}
				if (fileType == "effect")
				{
					// effect/fighter/[charcode]/...
					// effect/fighter/ = 15
					charcode = filePath.substr(15, filePath.substr(15).find('/'));
					vanillaEffectFiles[charcode].insert(filePath);
				}
				else if (fileType == "fighter")
				{
					// fighter/[charcode]
					// fighter/ = 8
					charcode = filePath.substr(8, filePath.substr(8).find('/'));

					// Kirby Copy File
					if (charcode == "kirby")
					{
						if (filePath.find("fighter/kirby/model/copy_") != string::npos)
						{
							// fighter/kirby/model/copy_[charcode]_...
							// fighter/kirby/model/copy_ = 25
							string charCopy = filePath.substr(25, filePath.find("_", 25) - 25);

							// fighter/kirby/motion/copy_[charcode]_bag/c[slot]/...
							size_t slotPos = filePath.find("/c0");
							if (slotPos != string::npos)
							{
								string charSlot = filePath.substr(slotPos + 2, 2);
								vanillaKirbyFiles[charCopy][charSlot].insert(filePath);
							}

							continue;
						}
						else if (filePath.find("fighter/kirby/motion/copy_") != string::npos)
						{
							// fighter/kirby/motion/copy_[charcode]_...
							// fighter/kirby/motion/copy_ = 26
							string charCopy = filePath.substr(26, filePath.find("_", 26) - 26);

							int slotPos = filePath.find("/c0");
							if (slotPos != string::npos)
							{
								string charSlot = filePath.substr(slotPos + 2, 2);
								addedKirbyFiles[charCopy][charSlot].insert(filePath);
							}

							continue;
						}
					}

					string slot;

					// fighter/[charcode/[filetype]/[character part]/c[slot]/...
					size_t cPos = filePath.find("/c0");
					if (cPos != string::npos)
					{
						// fighter/[charcode/[filetype]/[character part]/c[slot]/...
						slot = filePath.substr(cPos + 2, 2);
					}
					else
					{
						// TODO: Solve mii issues
						// log->LogText("> Invalid Vanilla Fighter File: " + filePath);
						continue;
					}

					if (filePath.find("/motion/") != string::npos)
					{
						addedFiles[charcode][slot].insert(filePath);
					}
					else
					{
						vanillaFiles[charcode][slot].insert(filePath);
					}
				}
				else if (fileType == "result")
				{
					// fighter/[charcode]
					// fighter/ = 8
					charcode = filePath.substr(8, filePath.substr(8).find('/'));

					string slot;

					// fighter/[charcode/[filetype]/[character part]/c[slot]/...
					size_t cPos = filePath.find("/c0");
					if (cPos != string::npos)
					{
						// fighter/[charcode/[filetype]/[character part]/c[slot]/...
						slot = filePath.substr(cPos + 2, 2);
					}
					else
					{
						// TODO: Solve mii issues
						// log->LogText("> Invalid Vanilla Fighter File: " + filePath);
						continue;
					}

					if (filePath.find("/motion/") != string::npos)
					{
						addedResultFiles[charcode][slot].insert(filePath);
					}
					else
					{
						vanillaResultFiles[charcode][slot].insert(filePath);
					}
				}
				else if (fileType == "camera")
				{
					// camera/fighter/[charcode]/...
					// camera/fighter/ = 15
					charcode = filePath.substr(15, filePath.substr(15).find('/'));

					// camera/fighter/[charcode]/c[slot]/
					// camera/fighter/ + /c = 17
					string slot = filePath.substr(17 + charcode.size(), 2);

					cameraFiles[charcode][slot].insert(filePath);
				}
			}
		}
		else
		{
			log->LogText("> " + ("files/" + fileType + ".data") + " could not be opened!");
			file.clear();
			continue;
		}

		file.close();
	}
}

void SmashData::addData(string charcode, string fileType, string slot, string file)
{
	for (int i = 0; i < charcode.length(); i++)
	{
		charcode[i] = tolower(charcode[i]);
	}

	if (charcode == "popo" || charcode == "nana")
	{
		charcode = "ice_climber";
	}

	if (charNames.find(charcode) == charNames.end())
	{
		charCodes[charcode] = charcode;
		charNames[charcode] = charcode;
	}

	std::replace(file.begin(), file.end(), '\\', '/');

	mod[charcode][fileType][slot].insert(file);
}

void SmashData::readData(string wxPath)
{
	rootPath = wxPath;
	std::replace(rootPath.begin(), rootPath.end(), '\\',  '/');

	for (const auto& i : fs::directory_iterator(rootPath))
	{
		string fileType = i.path().filename().string();

		if (fileType == "effect")
		{
			for (const auto& j : fs::directory_iterator(i))
			{
				if (j.path().filename() == "fighter")
				{
					// Folder = [charcode] (i.e. ike, koopa, etc.)
					for (const auto& k : fs::directory_iterator(j))
					{
						string charcode = k.path().filename().string();

						// Effect Files/Folders
						if (k.is_directory())
						{
							for (const auto& l : fs::directory_iterator(k))
							{
								string effectFile = l.path().filename().string();
								string slot;

								// effectFile is an eff file
								if (effectFile.rfind(".eff") != std::string::npos)
								{
									// One-Slotted eff file
									// ef_[charcode]_c[slot].eff
									// ef_ + _c + .eff = 9
									if (effectFile.size() > (9 + charcode.size()))
									{
										// ef_[charcode]_c[slot].eff
										// ef_ + _c = 5
										slot = effectFile.substr(5 + charcode.size(), effectFile.find(".eff") - (5 + charcode.size()));
									}
									// effectFile is not One-Slotted
									else
									{
										slot = "all";
									}

								}
								// effectFile is a folder
								else
								{
									// model folder requires special treatment
									if (effectFile == "model")
									{
										for (const auto& m : fs::directory_iterator(l))
										{
											string effectFile = m.path().filename().string();
											auto cPos = effectFile.rfind("_c");

											// Folder is one slotted
											if (cPos != string::npos && cPos != effectFile.find("_c"))
											{
												slot = effectFile.substr(cPos + 2);
											}
											else
											{
												slot = "all";
											}

											addData(charcode, fileType, slot, m.path().string());
										}

										continue;
									}
									// Character specific folder, likely a Trail folder (One-Slotted)
									else if(effectFile.find("_c") != string::npos)
									{
										// [character_specific_effect]_c[slot]
										// _c = 2
										slot = effectFile.substr(effectFile.rfind("_c") + 2);
									}
									// Character specific folder, likely a Trail folder (NOT One-Slotted)
									else
									{
										slot = "all";
									}
								}

								string file = l.path().string();

								addData(charcode, fileType, slot, file);
							}
						}
					}
				}
			}
		}
		else if (fileType == "fighter")
		{
			// Folder = [charcode] (i.e. ike, koopa, etc.)
			for (const auto& j : fs::directory_iterator(i))
			{
				string charcode = j.path().filename().string();

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
											string slot = m.path().filename().string().substr(1);
											string file = m.path().string();

											addData(charcode, fileType, slot, file);
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
									// vc_[charcode] || se_[charcode]
									// vc_ = 3       || se_ = 3
									size_t dotPos = filename.rfind('.');

									// Slotted
									if (dotPos != string::npos)
									{
										if (isdigit(filename[dotPos - 1]))
										{
											string charcode = filename.substr(3, filename.substr(3).find("_"));

											// [...]_c[slot].[filetype]
											// _c = 2
											size_t cPos = filename.rfind("_c");

											if (cPos != string::npos)
											{
												string slot = filename.substr(cPos + 2, filename.rfind(".") - cPos - 2);
												addData(charcode, fileType, slot, file);
											}
											else
											{
												log->LogText("> " + filename + "'s slot could not be determined!");
											}
										}
										else
										{
											string charcode = filename.substr(3, dotPos - 3);
											addData(charcode, fileType, "all", file);
										}
									}
									else
									{
										log->LogText("> " + filename + "is not a valid sound file!");
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
									// File = chara_[x]_[charcode]_[slot].bntx
									for (const auto& m : fs::directory_iterator(l))
									{
										string charcode = m.path().filename().string().substr(m.path().filename().string().find('_', 6) + 1);

										int underscoreIndex = charcode.rfind('_');

										if (underscoreIndex != string::npos)
										{
											string slot = charcode.substr(underscoreIndex + 1, 2);
											charcode = charcode.substr(0, underscoreIndex);

											if (charcode == "eflame_only" || charcode == "eflame_first")
											{
												charcode = "eflame";
											}
											else if (charcode == "elight_only" || charcode == "elight_first")
											{
												charcode = "elight";
											}

											string file = m.path().string();
											addData(charcode, fileType, slot, file);
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
			// log->LogText("> " + fileType + " is currently not supported, no changes will be made to " + fileType);
			continue;
		}
	}
}

// Mod Getters
wxArrayString SmashData::getCharacters() const
{
	wxArrayString characters;
	
	for (auto i = mod.begin(); i != mod.end(); i++)
	{
		if (charNames.find(i->first) != charNames.end())
		{
			characters.Add(charNames.find(i->first)->second);
		}
		else
		{
			log->LogText("> " + i->first + "'s name could not be found!");
		}
	}

	std::sort(characters.begin(), characters.end());

	return characters;
}

wxArrayString SmashData::getFileTypes(string charcode) const
{
	wxArrayString fileTypes;

	auto charIter = mod.find(charcode);

	if (charIter != mod.end())
	{
		for (auto i = charIter->second.begin(); i != charIter->second.end(); i++)
		{
			fileTypes.Add(i->first);
		}
	}

	return fileTypes;
}

wxArrayString SmashData::getSlots(string charcode) const
{
	// Stores slots, later converted to wxArrayString
	set<wxString> slotsSet;

	auto charIter = mod.find(charcode);
	bool special = false;

	if (charcode == "popo" || charcode == "nana")
	{
		charIter = mod.find("ice_climber");
		special = true;
	}

	if (charIter != mod.end())
	{
		for (int i = 0; i < fileTypes.size(); i++)
		{
			auto fileTypeIter = charIter->second.find(fileTypes[i]);

			if (fileTypeIter != charIter->second.end())
			{
				for (auto j = fileTypeIter->second.begin(); j != fileTypeIter->second.end(); j++)
				{
					if (special)
					{
						for (auto k = j->second.begin(); k != j->second.end(); k++)
						{
							string label;

							if (fileTypeIter->first == "effect" || fileTypeIter->first == "fighter")
							{
								label = "/" + charcode + "/";
							}
							else if (fileTypeIter->first == "ui" || fileTypeIter->first == "sound")
							{
								label = "_" + charcode + "_";
							}

							if (k->rfind(label) != string::npos)
							{
								slotsSet.insert(j->first);
								break;
							}
						}
					}
					else
					{
						slotsSet.insert(j->first);
					}
				}
			}
		}
	}

	wxArrayString slots;

	for (auto i = slotsSet.begin(); i != slotsSet.end(); i++)
	{
		slots.Add(i->ToStdString());
	}

	return slots;
}

wxArrayString SmashData::getSlots(string charcode, string fileType) const
{
	wxArrayString slots;

	auto charIter = mod.find(charcode);
	bool special = false;

	if (charcode == "popo" || charcode == "nana")
	{
		charIter = mod.find("ice_climber");
		special = true;
	}

	if (charIter != mod.end())
	{
		auto fileTypeIter = charIter->second.find(fileType);

		if (fileTypeIter != charIter->second.end())
		{
			for (auto i = fileTypeIter->second.begin(); i != fileTypeIter->second.end(); i++)
			{
				if (special)
				{
					for (auto j = i->second.begin(); j != i->second.end(); j++)
					{
						string label;

						if (fileTypeIter->first == "effect" || fileTypeIter->first == "fighter")
						{
							label = "/" + charcode + "/";
						}
						else if (fileTypeIter->first == "ui" || fileTypeIter->first == "sound")
						{
							label = "_" + charcode + "_";
						}

						if (j->rfind(label) != string::npos)
						{
							slots.Add(i->first);
							break;
						}
					}
				}
				else
				{
					slots.Add(i->first);
				}
			}
		}
	}

	return slots;
}

wxArrayString SmashData::getSlots(string charcode, wxArrayString fileTypes) const
{
	wxArrayString initSlots;
	wxArrayString slots;

	auto charIter = mod.find(charcode);
	bool special = false;

	if (charcode == "popo" || charcode == "nana")
	{
		charIter = mod.find("ice_climber");
		special = true;
	}
	else
	{

	}

	// Get slots for one file type
	if(!fileTypes.empty() && charIter != mod.end())
	{
		// Priorotize file types besides "effect" (it has "all")
		if (fileTypes[0] == "effect" && fileTypes.size() > 1)
		{
			initSlots = getSlots(charcode, fileTypes[1].ToStdString());
		}
		else
		{
			initSlots = getSlots(charcode, fileTypes[0].ToStdString());
		}

		if (fileTypes.size() <= 1)
		{
			return initSlots;
		}
	}
	else
	{
		return slots;
	}

	// Add to slots if available in each fileType
	for (int i = 0; i < initSlots.size(); i++)
	{
		bool foundInAll = true;

		for (int j = 0; j < fileTypes.size(); j++)
		{
			auto fileTypeIter = charIter->second.find(fileTypes[j].ToStdString());

			if (fileTypeIter != charIter->second.end())
			{
				auto slotIter = fileTypeIter->second.find(initSlots[i].ToStdString());

				// Slot from different fileType was not found in current fileType
				if (slotIter == fileTypeIter->second.end())
				{
					if (fileTypes[j] != "effect")
					{
						foundInAll = false;
					}
					// FileType is effect and an "all" version does not exist.
					else if (fileTypeIter->second.find("all") == fileTypeIter->second.end())
					{
						foundInAll = false;
					}
				}
				else if (special)
				{
					for (auto k = slotIter->second.begin(); k != slotIter->second.end(); k++)
					{
						string label;

						if (fileTypeIter->first == "effect" || fileTypeIter->first == "fighter")
						{
							label = "/" + charcode + "/";
						}
						else if (fileTypeIter->first == "ui" || fileTypeIter->first == "sound")
						{
							label = "_" + charcode + "_";
						}

						if (k->rfind(label) != string::npos)
						{
							break;
						}
					}

					foundInAll = false;
				}
			}
			else
			{
				foundInAll = false;
			}
		}

		if (foundInAll)
		{
			slots.Add(initSlots[i]);
		}
	}

	return slots;
}

string SmashData::getBaseSlot(string charcode, string addSlot)
{
	if (stoi(addSlot) <= 7)
	{
		return addSlot;
	}
	else if (baseSlots.find(charcode) != baseSlots.end())
	{
		for (auto i = baseSlots[charcode].begin(); i != baseSlots[charcode].end(); i++)
		{
			if (i->second.find(addSlot) != i->second.end())
			{
				return i->first;
			}
		}
	}

	return "-1";
}

// Mod Verifiers
bool SmashData::hasFileType(string fileType) const
{
	for (auto i = mod.begin(); i != mod.end(); i++)
	{
		for (auto j = i->second.begin(); j != i->second.end(); j++)
		{
			if (j->first == fileType)
			{
				return true;
			}
		}
	}

	return false;
}

bool SmashData::hasAdditionalSlot() const
{
	for (auto i = mod.begin(); i != mod.end(); i++)
	{
		for (auto j = i->second.begin(); j != i->second.end(); j++)
		{
			for (auto k = j->second.begin(); k != j->second.end(); k++)
			{
				try
				{
					if (stoi(k->first) > 7)
					{
						// Ignore kirby slots that only contain copy files.
						if (i->first == "kirby" && j->first == "fighter")
						{
							bool additional = false;

							for (auto l = k->second.begin(); l != k->second.end(); l++)
							{
								if (l->find("copy_") == string::npos)
								{
									additional = true;
									break;
								}
							}

							if (additional)
							{
								return true;
							}
							else
							{
								continue;
							}
						}

						return true;
					}
				}
				catch(...)
				{
					// Likely "all" slot from effect. Ignore it.
				}
			}
		}
	}

	return false;
}

// TODO: Remove duplicate code
bool SmashData::hasAdditionalSlot(string charcode) const
{
	auto charIter = mod.find(charcode);

	if (charIter != mod.end())
	{
		for (auto i = charIter->second.begin(); i != charIter->second.end(); i++)
		{
			for (auto j = i->second.begin(); j != i->second.end(); j++)
			{
				try
				{
					if (stoi(j->first) > 7)
					{
						// Ignore kirby slots that only contain copy files.
						if (i->first == "kirby" && j->first == "fighter")
						{
							bool additional = false;

							for (auto k = j->second.begin(); k != j->second.end(); k++)
							{
								if (k->find("copy_") == string::npos)
								{
									additional = true;
									break;
								}
							}

							if (additional)
							{
								return true;
							}
							else
							{
								continue;
							}
						}

						return true;
					}
				}
				catch (...)
				{
					// Likely "all" slot from effect. Ignore it.
				}
			}
		}
	}

	return false;
}

bool SmashData::hasSlot(string charcode, string slot) const
{
	// Conver vector to wxArray
	wxArrayString fileTypes;

	for (int i = 0; i < this->fileTypes.size(); i++)
	{
		fileTypes.Add(this->fileTypes[i]);
	}

	// Filetypes is from the SmashData members
	return this->hasSlot(charcode, fileTypes, slot);
}

bool SmashData::hasSlot(string charcode, wxArrayString fileTypes, string slot) const
{
	auto charIter = mod.find(charcode);
	bool special = false;

	if (charcode == "popo" || charcode == "nana")
	{
		charIter = mod.find("ice_climber");
		special = true;
	}

	if (charIter != mod.end())
	{
		for (int i = 0; i < fileTypes.size(); i++)
		{
			auto fileTypeIter = charIter->second.find(fileTypes[i].ToStdString());

			if (fileTypeIter != charIter->second.end())
			{
				auto slotIter = fileTypeIter->second.find(slot);

				if (slotIter != fileTypeIter->second.end())
				{
					if (special)
					{
						for (auto j = slotIter->second.begin(); j != slotIter->second.end(); j++)
						{
							string label;

							if (fileTypeIter->first == "effect" || fileTypeIter->first == "fighter")
							{
								label = "/" + charcode + "/";
							}
							else if (fileTypeIter->first == "ui" || fileTypeIter->first == "sound")
							{
								label = "_" + charcode + "_";
							}

							if (j->rfind(label) != string::npos)
							{
								return true;
							}
						}
					}
					else
					{
						return true;
					}
				}
			}
		}
	}

	return false;
}

// Mod Modifiers
void SmashData::adjustFiles(string action, string charcode, wxArrayString fileTypes, string initSlot, string finalSlot)
{
	auto charIter = mod.find(charcode);

	if (charIter != mod.end())
	{
		for (int i = 0; i < fileTypes.size(); i++)
		{
			auto fileTypeIter = charIter->second.find(fileTypes[i].ToStdString());

			if (fileTypeIter != charIter->second.end())
			{
				auto slotIter = fileTypeIter->second.find(initSlot);

				if (slotIter != fileTypeIter->second.end())
				{
					for (auto j = slotIter->second.begin(); j != slotIter->second.end(); j++)
					{
						std::filesystem::path initPath(*j);
						std::filesystem::path finalPath(*j);

						if (fileTypes[i] == "effect")
						{
							// One-Slotted Effect
							if (initSlot != "all")
							{
								// eff file
								if (j->find(".eff") != std::string::npos)
								{
									string fileName = finalPath.filename().string();
									fileName = fileName.substr(0, fileName.rfind("_")) + "_c" + finalSlot + ".eff";
									finalPath.replace_filename(fileName);
								}
								// Effect_specific folder
								else
								{
									string folderName = finalPath.filename().string();
									folderName = folderName.substr(0, folderName.size() - initSlot.size()).append(finalSlot);
									finalPath.replace_filename(folderName);
								}
							}
							// NOT One-Slotted Effect
							else
							{
								initPath = *j;
								finalPath = *j;

								if (j->find(".eff") != std::string::npos)
								{
									// .eff = 4
									string fileName = finalPath.filename().string();
									fileName = fileName.substr(0, fileName.size() - 4) + "_c" + finalSlot + ".eff";
									finalPath.replace_filename(fileName);
								}
								else
								{
									finalPath.replace_filename(finalPath.filename().string() + "_c" + finalSlot);
								}
							}
						}
						else if (fileTypes[i] == "fighter")
						{
							finalPath.replace_filename("c" + finalSlot);
						}
						else if (fileTypes[i] == "sound")
						{
							// se_[charcode]_c[slot].[filetype]
							// vc_[charcode]_c[slot].[filetype]
							// vc_[charcode]_cheer_c[slot].[filetype]
							string fileName = finalPath.filename().string();
							size_t dotPos = fileName.find(".");
							
							if (isdigit(fileName[dotPos - 1]))
							{
								fileName = fileName.substr(0, fileName.rfind("_c") + 2) + finalSlot;
								fileName += initPath.extension().string();
							}
							else
							{
								fileName = fileName.substr(0, dotPos) + "_c" + finalSlot + initPath.extension().string();
							}

							finalPath.replace_filename(fileName);
						}
						else if (fileTypes[i] == "ui")
						{
							// chara_[x]_[charcode]_[slot].bntx
							// . + b + n + t + x = 5
							string fileName = finalPath.filename().string();
							fileName = fileName.substr(0, fileName.size() - initSlot.size() - 5) + finalSlot;
							fileName += initPath.extension().string();
							finalPath.replace_filename(fileName);
						}
						else
						{
							log->LogText("> Error! " + charcode + " has no " + fileTypes[i] + " file type!");
							return;
						}

						try
						{
							if (action == "move")
							{
								std::filesystem::rename(initPath, finalPath);
							}
							else if (action == "duplicate")
							{
								std::filesystem::copy(initPath, finalPath, fs::copy_options::recursive);
							}
							else if (action == "delete")
							{
								std::filesystem::remove_all(initPath);
							}
							else
							{
								log->LogText("> Error! " + action + " not found!");
								return;
							}

							if (action != "delete")
							{
								string resultPath = finalPath.string();
								std::replace(resultPath.begin(), resultPath.end(), '\\', '/');

								mod[charcode][fileTypes[i].ToStdString()][finalSlot].insert(resultPath);
							}
						}
						catch (...)
						{
							if (action == "move")
							{
								log->LogText("> Error! " + initPath.string() + " could not be renamed!");
							}
							else if (action == "duplicate")
							{
								log->LogText("> Error! " + initPath.string() + " could not be copied!");
							}
							else if (action == "delete")
							{
								log->LogText("Error! " + initPath.string() + " could not be deleted!");
							}
							else
							{
								log->LogText("> Error! " + action + " not found!");
							}

							return;
						}
					}
				}
			}
			else
			{
				continue;
			}

			if (action == "move")
			{
				mod[charcode][fileTypes[i].ToStdString()].extract(initSlot);
			}
			else if (action == "delete")
			{
				mod[charcode][fileTypes[i].ToStdString()].extract(initSlot);

				if (mod[charcode][fileTypes[i].ToStdString()].empty())
				{
					mod[charcode].extract(fileTypes[i].ToStdString());

					if (fileTypes[i] == "fighter")
					{
						std::filesystem::remove_all((rootPath + "/fighter/" + charcode));
					}

					// Delete empty folders
					// TODO: Check folder before deletion
					/*
					if (!hasFileType(fileTypes[i]))
					{
						std::filesystem::remove_all((rootPath + "/" + fileTypes[i]).ToStdString());
					}
					*/
				}

				if (mod[charcode].empty())
				{
					mod.extract(charcode);
				}
			}
		}
	}
	else
	{
		log->LogText("> Error! " + charcode + " does not exist!");
	}

	if (action == "move")
	{
		if (finalSlot == "all")
		{
			log->LogText("> Success! " + charcode + "'s default effect was moved to c" + finalSlot + "!");
		}
		else
		{
			log->LogText("> Success! " + charcode + "'s c" + initSlot + " was moved to c" + finalSlot + "!");
		}
	}
	else if (action == "duplicate")
	{
		if (finalSlot == "all")
		{
			log->LogText("> Success! " + charcode + "'s default effect was duplicated to c" + finalSlot + "!");
		}
		else
		{
			log->LogText("> Success! " + charcode + "'s c" + initSlot + " was duplicated to c" + finalSlot + "!");
		}
	}
	else if (action == "delete")
	{
		if (finalSlot == "all")
		{
			log->LogText("> Success! " + charcode + "'s default effect was deleted!");
		}
		else
		{
			log->LogText("> Success! " + charcode + "'s c" + initSlot + " was deleted!");
		}
	}
	else
	{
		if (finalSlot == "all")
		{
			log->LogText("> Success ? " + charcode + "'s default effect was " + action + "-ed to c" + finalSlot + "!");
		}
		else
		{
			log->LogText("> Success? " + charcode + "'s c" + initSlot + " was " + action + "-ed to c" + finalSlot + "!");
		}
	}
}

// Config Getters
map<string, set<string>> SmashData::getAdditionalSlots()
{
	map<string, set<string>> additionalSlots;

	for (auto i = mod.begin(); i != mod.end(); i++)
	{
		for (auto j = i->second.begin(); j != i->second.end(); j++)
		{
			for (auto k = j->second.begin(); k != j->second.end(); k++)
			{
				try
				{
					if (std::stoi(k->first) > 7)
					{
						// Ignore kirby slots that only contain copy files.
						if (i->first == "kirby" && j->first == "fighter")
						{
							for (auto l = k->second.begin(); l != k->second.end(); l++)
							{
								if (l->find("copy_") == string::npos)
								{
									break;
								}
							}

							continue;
						}

						additionalSlots[i->first].insert(k->first);
					}

					// TODO: Possibly incorporate base slot selection for cloud, ike, etc.
					// NOTE: Might not be needed as newDirFile currently accounts for all files of choosen slot
				}
				catch (...)
				{
					// Invalid slot (likely "all" from effect)
				}
			}
		}
	}

	return additionalSlots;
}

void SmashData::getNewDirSlots
(
	// charcode, base-slot, actual-slots
	const map<string, map<string, set<string>>>& baseSlots,
	// files
	vector<string>& newDirInfos,
	// files
	vector<string>& newDirInfosBase,
	// charcode, slot, file, files
	map<string, map<string, map<string, set<string>>>>& shareToVanilla,
	// charcode, slot, file, files
	map<string, map<string, map<string, set<string>>>>& shareToAdded,
	// charcode, slot, section-label, files
	map<string, map<string, map<string, set<string>>>>& newDirFiles
)
{
	string charcode = "";
	bool hasElement = mod.find("element") != mod.end();
	bool hasKirby = mod.find("kirby") != mod.end();
	auto i = mod.begin();

	map<string, set<string>> kirbyFiles;

	// Get Kirby files
	if (hasKirby)
	{
		for (auto k = mod["kirby"]["fighter"].begin(); k != mod["kirby"]["fighter"].end(); k++)
		{
			for (auto l = k->second.begin(); l != k->second.end(); l++)
			{
				if (i->first == "kirby" && l->find("/copy_") != string::npos)
				{
					for (const auto& m : fs::directory_iterator(*l))
					{
						// Convert system path to smash path
						string path = m.path().string().substr(rootPath.size() + 1);
						std::replace(path.begin(), path.end(), '\\', '/');

						kirbyFiles[k->first].insert(path);
					}
				}
			}
		}
	}

	while(i != mod.end())
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
		else if(hasElement || (charcode != "element" || i->first != "eflame"))
		{
			if (charcode == "koopag")
			{
				continue;
			}

			charcode = i->first;
		}

		// Get all slots for charcode
		auto charSlots = getSlots(charcode);

		bool charHasFighter = i->second.find("fighter") != i->second.end();
		bool charHasEffect = i->second.find("effect") != i->second.end();
		bool charHasSound = i->second.find("sound") != i->second.end();

		if (!hasElement && charcode == "element" && i->first == "eflame")
		{
			charSlots = getSlots(i->first);
			charHasFighter = false;
			charHasEffect = false;
			charHasSound = false;
		}

		for (auto j = charSlots.begin(); j != charSlots.end(); j++)
		{
			// Skip all slot
			if (*j == "all")
			{
				continue;
			}

			bool slotHasFighter = charHasFighter && mod[i->first]["fighter"].find(j->ToStdString()) != mod[i->first]["fighter"].end();
			bool slotHasEffect = charHasEffect && mod[i->first]["effect"].find(j->ToStdString()) != mod[i->first]["effect"].end();
			bool slotHasSound = charHasSound && mod[i->first]["sound"].find(j->ToStdString()) != mod[i->first]["sound"].end();

			bool additionalSlot = stoi(j->ToStdString()) > 7;

			// Kirby case for additionalSlot
			if (charcode == "kirby")
			{
				bool copyOnly = true;

				for (auto k = mod[charcode]["fighter"][j->ToStdString()].begin(); k != mod[charcode]["fighter"][j->ToStdString()].end(); k++)
				{
					if (k->find("copy_") == string::npos)
					{
						copyOnly = false;
					}
				}

				if (copyOnly)
				{
					additionalSlot = false;
				}
			}

			string baseSlot = "";

			set<string> fighterFiles;
			set<string> effectFiles;

			// Read fighter files
			if (slotHasFighter)
			{
				for (auto k = mod[i->first]["fighter"][j->ToStdString()].begin(); k != mod[i->first]["fighter"][j->ToStdString()].end(); k++)
				{
					// Skip popo or nana's file reading if their name is not within the path
					if (i->first == "ice_climber" && k->find("fighter/" + charcode + "/") == string::npos)
					{
						continue;
					}

					for (const auto& l : fs::directory_iterator(*k))
					{
						// Convert system path to smash path
						string path = l.path().string().substr(rootPath.size() + 1);
						std::replace(path.begin(), path.end(), '\\', '/');

						fighterFiles.insert(path);
					}
				}

				if (i->first == "ice_climber" && fighterFiles.empty())
				{
					slotHasFighter = false;
				}
			}

			// Read effect files
			if (slotHasEffect)
			{
				for (auto k = mod[i->first]["effect"][j->ToStdString()].begin(); k != mod[i->first]["effect"][j->ToStdString()].end(); k++)
				{
					// Skip popo or nana's file reading if their name is not within the path
					if (i->first == "ice_climber" && k->find("effect/fighter/" + charcode + "/") == string::npos)
					{
						continue;
					}

					// [...].eff
					// .eff = 4
					if (k->substr(k->size() - 4, 4) == ".eff")
					{
						effectFiles.insert(k->substr(rootPath.size() + 1));
					}
					else
					{
						for (const auto& l : fs::directory_iterator(*k))
						{
							string path = l.path().string().substr(rootPath.size() + 1);
							std::replace(path.begin(), path.end(), '\\', '/');

							effectFiles.insert(path);
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
				auto baseCharIter = baseSlots.find(i->first);

				// Get base slot
				for (auto k = baseCharIter->second.begin(); k != baseCharIter->second.end(); k++)
				{
					if (k->second.find(j->ToStdString()) != k->second.end())
					{
						baseSlot = k->first;
						break;
					}
				}

				// Base slot was not found
				if (baseSlot == "")
				{
					log->LogText("> Error: Base slot for " + i->first + "'s c" + j->ToStdString() + " slot could not be found!");
					return;
				}
				else
				{
					// Add newDirInfos
					if (charcode == "jack")
					{
						newDirInfos.push_back("\"fighter/" + charcode + "/append/c" + j->ToStdString() + "\"");
					}
					newDirInfos.push_back("\"fighter/" + charcode + "/c" + j->ToStdString() + "\"");
					newDirInfos.push_back("\"fighter/" + charcode + "/camera/c" + j->ToStdString() + "\"");
					if (charcode != "nana" && charcode != "kirby")
					{
						newDirInfos.push_back("\"fighter/" + charcode + "/kirbycopy/c" + j->ToStdString() + "\"");
					}
					newDirInfos.push_back("\"fighter/" + charcode + "/movie/c" + j->ToStdString() + "\"");
					newDirInfos.push_back("\"fighter/" + charcode + "/result/c" + j->ToStdString() + "\"");

					// Add newDirInfosBase
					newDirInfosBase.push_back("\"fighter/" + charcode + "/c" + j->ToStdString() + "/camera\": \"fighter/" + charcode + "/c" + baseSlot + "/camera\"");
					newDirInfosBase.push_back("\"fighter/" + charcode + "/c" + j->ToStdString() + "/cmn\": \"fighter/" + charcode + "/c" + baseSlot + "/cmn\"");
					if (charcode != "nana" && charcode != "kirby")
					{
						newDirInfosBase.push_back("\"fighter/" + charcode + "/kirbycopy/c" + j->ToStdString() + "/bodymotion\": \"fighter/" + charcode + "/kirbycopy/c" + baseSlot + "/bodymotion\"");
						newDirInfosBase.push_back("\"fighter/" + charcode + "/kirbycopy/c" + j->ToStdString() + "/cmn\": \"fighter/" + charcode + "/kirbycopy/c" + baseSlot + "/cmn\"");
						newDirInfosBase.push_back("\"fighter/" + charcode + "/kirbycopy/c" + j->ToStdString() + "/sound\": \"fighter/" + charcode + "/kirbycopy/c" + baseSlot + "/sound\"");
					}

					// Add camera, fighter, and kirby files to newDirFiles
					vector fileTypes = { &cameraFiles , &vanillaFiles, &addedFiles, &vanillaResultFiles, &addedResultFiles, &vanillaKirbyFiles, &addedKirbyFiles };
					for (auto k = 0; k < fileTypes.size(); k++)
					{
						// Go through camera/vanilla/added/vanillaKirby/addedKirby files
						for (auto l = (*fileTypes[k])[charcode][baseSlot].begin(); l != (*fileTypes[k])[charcode][baseSlot].end(); l++)
						{
							string path = *l;
							int cPos = path.find("/c" + baseSlot);

							if (cPos != string::npos)
							{
								string label;
								path.replace(cPos, 2 + baseSlot.size(), "/c" + *j);

								// Camera Files
								if (k == 0)
								{
									label = "\"fighter/" + charcode + "/camera/c" + *j + "\"";
									shareToAdded[i->first][baseSlot]["\"" + *l + "\""].insert("\"" + path + "\"");
								}
								// Vanilla/Added Files
								else if (k < 3)
								{
									label = "\"fighter/" + charcode + "/c" + *j + "\"";

									if (!slotHasFighter || fighterFiles.find(path) == fighterFiles.end())
									{
										if (k == 1)
										{
											shareToVanilla[i->first][baseSlot]["\"" + *l + "\""].insert("\"" + path + "\"");
										}
										else
										{
											shareToAdded[i->first][baseSlot]["\"" + *l + "\""].insert("\"" + path + "\"");
										}
									}
								}
								// Vanilla/Added Result Files
								else if (k < 5)
								{
									label = "\"fighter/" + charcode + "/result/c" + j->ToStdString() + "\"";

									if (!slotHasFighter || fighterFiles.find(path) == fighterFiles.end())
									{
										if (k == 3)
										{
											shareToVanilla[i->first][baseSlot]["\"" + *l + "\""].insert("\"" + path + "\"");
										}
										else
										{
											shareToAdded[i->first][baseSlot]["\"" + *l + "\""].insert("\"" + path + "\"");
										}
									}
								}
								// Vanilla/Added Kirby Files
								else
								{
									label = "\"fighter/" + charcode + "/kirbycopy/c" + *j + "\"";

									if (!hasKirby || kirbyFiles.empty() || kirbyFiles.find(j->ToStdString()) == kirbyFiles.end() || kirbyFiles[j->ToStdString()].find(path) == kirbyFiles[j->ToStdString()].end())
									{
										if (k == 5)
										{
											shareToVanilla[i->first][baseSlot]["\"" + *l + "\""].insert("\"" + path + "\"");
										}
										else
										{
											shareToAdded[i->first][baseSlot]["\"" + *l + "\""].insert("\"" + path + "\"");
										}
									}
								}

								newDirFiles[i->first][j->ToStdString()][label].insert("\"" + path + "\"");
							}
							else
							{
								log->LogText("> " + *l + "'s slot could not be determined");
								return;
							}
						}
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

					// Add sound files
					// TODO: Implement REAL Vanilla check
					if (slotHasSound)
					{
						if (mod[i->first]["sound"][j->ToStdString()].find(rootPath + "/sound/bank/fighter/se_" + charcode + "_c" + j->ToStdString() + ".nus3audio") == mod[i->first]["sound"][j->ToStdString()].end())
						{
							shareToVanilla[i->first][baseSlot]["\"sound/bank/fighter/se_" + charcode + "_c" + baseSlot + ".nus3audio\""].insert("\"sound/bank/fighter/se_" + charcode + "_c" + j->ToStdString() + ".nus3audio\"");
						}
						if (mod[i->first]["sound"][j->ToStdString()].find(rootPath + "/sound/bank/fighter/se_" + charcode + "_c" + j->ToStdString() + ".nus3bank") == mod[i->first]["sound"][j->ToStdString()].end())
						{
							shareToVanilla[i->first][baseSlot]["\"sound/bank/fighter/se_" + charcode + "_c" + baseSlot + ".nus3bank\""].insert("\"sound/bank/fighter/se_" + charcode + "_c" + j->ToStdString() + ".nus3bank\"");
						}
						if (mod[i->first]["sound"][j->ToStdString()].find(rootPath + "/sound/bank/fighter/se_" + charcode + "_c" + j->ToStdString() + ".tonelabel") == mod[i->first]["sound"][j->ToStdString()].end())
						{
							shareToVanilla[i->first][baseSlot]["\"sound/bank/fighter/se_" + charcode + "_c" + baseSlot + ".tonelabel\""].insert("\"sound/bank/fighter/se_" + charcode + "_c" + j->ToStdString() + ".tonelabel\"");
						}
						if (mod[i->first]["sound"][j->ToStdString()].find(rootPath + "/sound/bank/fighter_voice/vc_" + charcode + "_c" + j->ToStdString() + ".nus3audio") == mod[i->first]["sound"][j->ToStdString()].end())
						{
							shareToVanilla[i->first][baseSlot]["\"sound/bank/fighter_voice/vc_" + charcode + "_c" + baseSlot + ".nus3audio\""].insert("\"sound/bank/fighter_voice/vc_" + charcode + "_c" + j->ToStdString() + ".nus3audio\"");
						}
						if (mod[i->first]["sound"][j->ToStdString()].find(rootPath + "/sound/bank/fighter_voice/vc_" + charcode + "_c" + j->ToStdString() + ".nus3bank") == mod[i->first]["sound"][j->ToStdString()].end())
						{
							shareToVanilla[i->first][baseSlot]["\"sound/bank/fighter_voice/vc_" + charcode + "_c" + baseSlot + ".nus3bank\""].insert("\"sound/bank/fighter_voice/vc_" + charcode + "_c" + j->ToStdString() + ".nus3bank\"");
						}
						if (mod[i->first]["sound"][j->ToStdString()].find(rootPath + "/sound/bank/fighter_voice/vc_" + charcode + "_c" + j->ToStdString() + ".tonelabel") == mod[i->first]["sound"][j->ToStdString()].end())
						{
							shareToVanilla[i->first][baseSlot]["\"sound/bank/fighter_voice/vc_" + charcode + "_c" + baseSlot + ".tonelabel\""].insert("\"sound/bank/fighter_voice/vc_" + charcode + "_c" + j->ToStdString() + ".tonelabel\"");
						}

						if (charcode != "nana")
						{
							if (mod[i->first]["sound"][j->ToStdString()].find(rootPath + "/sound/bank/fighter_voice/vc_" + charcode + "_cheer_c" + j->ToStdString() + ".nus3audio") == mod[i->first]["sound"][j->ToStdString()].end())
							{
								shareToVanilla[i->first][baseSlot]["\"sound/bank/fighter_voice/vc_" + charcode + "_cheer_c" + baseSlot + ".nus3audio\""].insert("\"sound/bank/fighter_voice/vc_" + charcode + "_cheer_c" + j->ToStdString() + ".nus3audio\"");
							}
							if (mod[i->first]["sound"][j->ToStdString()].find(rootPath + "/sound/bank/fighter_voice/vc_" + charcode + "_cheer_c" + j->ToStdString() + ".nus3bank") == mod[i->first]["sound"][j->ToStdString()].end())
							{
								shareToVanilla[i->first][baseSlot]["\"sound/bank/fighter_voice/vc_" + charcode + "_cheer_c" + baseSlot + ".nus3bank\""].insert("\"sound/bank/fighter_voice/vc_" + charcode + "_cheer_c" + j->ToStdString() + ".nus3bank\"");
							}
							if (mod[i->first]["sound"][j->ToStdString()].find(rootPath + "/sound/bank/fighter_voice/vc_" + charcode + "_cheer_c" + j->ToStdString() + ".tonelabel") == mod[i->first]["sound"][j->ToStdString()].end())
							{
								shareToVanilla[i->first][baseSlot]["\"sound/bank/fighter_voice/vc_" + charcode + "_cheer_c" + baseSlot + ".tonelabel\""].insert("\"sound/bank/fighter_voice/vc_" + charcode + "_cheer_c" + j->ToStdString() + ".tonelabel\"");
							}
						}
					}
					else
					{
						shareToVanilla[i->first][baseSlot]["\"sound/bank/fighter/se_" + charcode + "_c" + baseSlot + ".nus3audio\""].insert("\"sound/bank/fighter/se_" + charcode + "_c" + j->ToStdString() + ".nus3audio\"");
						shareToVanilla[i->first][baseSlot]["\"sound/bank/fighter/se_" + charcode + "_c" + baseSlot + ".nus3bank\""].insert("\"sound/bank/fighter/se_" + charcode + "_c" + j->ToStdString() + ".nus3bank\"");
						shareToVanilla[i->first][baseSlot]["\"sound/bank/fighter/se_" + charcode + "_c" + baseSlot + ".tonelabel\""].insert("\"sound/bank/fighter/se_" + charcode + "_c" + j->ToStdString() + ".tonelabel\"");
						shareToVanilla[i->first][baseSlot]["\"sound/bank/fighter_voice/vc_" + charcode + "_c" + baseSlot + ".nus3audio\""].insert("\"sound/bank/fighter_voice/vc_" + charcode + "_c" + j->ToStdString() + ".nus3audio\"");
						shareToVanilla[i->first][baseSlot]["\"sound/bank/fighter_voice/vc_" + charcode + "_c" + baseSlot + ".nus3bank\""].insert("\"sound/bank/fighter_voice/vc_" + charcode + "_c" + j->ToStdString() + ".nus3bank\"");
						shareToVanilla[i->first][baseSlot]["\"sound/bank/fighter_voice/vc_" + charcode + "_c" + baseSlot + ".tonelabel\""].insert("\"sound/bank/fighter_voice/vc_" + charcode + "_c" + j->ToStdString() + ".tonelabel\"");
						
						if (charcode != "nana")
						{
							shareToVanilla[i->first][baseSlot]["\"sound/bank/fighter_voice/vc_" + charcode + "_cheer_c" + baseSlot + ".nus3audio\""].insert("\"sound/bank/fighter_voice/vc_" + charcode + "_cheer_c" + j->ToStdString() + ".nus3audio\"");
							shareToVanilla[i->first][baseSlot]["\"sound/bank/fighter_voice/vc_" + charcode + "_cheer_c" + baseSlot + ".nus3bank\""].insert("\"sound/bank/fighter_voice/vc_" + charcode + "_cheer_c" + j->ToStdString() + ".nus3bank\"");
							shareToVanilla[i->first][baseSlot]["\"sound/bank/fighter_voice/vc_" + charcode + "_cheer_c" + baseSlot + ".tonelabel\""].insert("\"sound/bank/fighter_voice/vc_" + charcode + "_cheer_c" + j->ToStdString() + ".tonelabel\"");
						}
					}

					newDirFiles[i->first][j->ToStdString()]["\"fighter/" + charcode + "/c" + j->ToStdString() + "\""].insert("\"sound/bank/fighter/se_" + charcode + "_c" + j->ToStdString() + ".nus3audio\"");
					newDirFiles[i->first][j->ToStdString()]["\"fighter/" + charcode + "/c" + j->ToStdString() + "\""].insert("\"sound/bank/fighter/se_" + charcode + "_c" + j->ToStdString() + ".nus3bank\"");
					newDirFiles[i->first][j->ToStdString()]["\"fighter/" + charcode + "/c" + j->ToStdString() + "\""].insert("\"sound/bank/fighter/se_" + charcode + "_c" + j->ToStdString() + ".tonelabel\"");
					newDirFiles[i->first][j->ToStdString()]["\"fighter/" + charcode + "/c" + j->ToStdString() + "\""].insert("\"sound/bank/fighter_voice/vc_" + charcode + "_c" + j->ToStdString() + ".nus3audio\"");
					newDirFiles[i->first][j->ToStdString()]["\"fighter/" + charcode + "/c" + j->ToStdString() + "\""].insert("\"sound/bank/fighter_voice/vc_" + charcode + "_c" + j->ToStdString() + ".nus3bank\"");
					newDirFiles[i->first][j->ToStdString()]["\"fighter/" + charcode + "/c" + j->ToStdString() + "\""].insert("\"sound/bank/fighter_voice/vc_" + charcode + "_c" + j->ToStdString() + ".tonelabel\"");

					if (charcode != "nana")
					{
						newDirFiles[i->first][j->ToStdString()]["\"fighter/" + charcode + "/c" + j->ToStdString() + "\""].insert("\"sound/bank/fighter_voice/vc_" + charcode + "_cheer_c" + j->ToStdString() + ".nus3audio\"");
						newDirFiles[i->first][j->ToStdString()]["\"fighter/" + charcode + "/c" + j->ToStdString() + "\""].insert("\"sound/bank/fighter_voice/vc_" + charcode + "_cheer_c" + j->ToStdString() + ".nus3bank\"");
						newDirFiles[i->first][j->ToStdString()]["\"fighter/" + charcode + "/c" + j->ToStdString() + "\""].insert("\"sound/bank/fighter_voice/vc_" + charcode + "_cheer_c" + j->ToStdString() + ".tonelabel\"");
					}

					// Add movie (and append) files
					// TODO: Figure out which characters need these
					if (i->first == "edge")
					{
						newDirFiles[i->first][j->ToStdString()]["\"fighter/" + charcode + "/movie/c" + j->ToStdString() + "\""].insert("\"prebuilt:/movie/fighter/edge/c" + j->ToStdString() + "/final_00.h264\"");
						shareToVanilla[i->first][baseSlot]["\"prebuilt:/movie/fighter/edge/c" + baseSlot + "/final_00.h264\""].insert("\"prebuilt:/movie/fighter/edge/c" + j->ToStdString() + "/final_00.h264\"");
					}
					else if (i->first == "jack")
					{
						newDirFiles[i->first][j->ToStdString()]["\"fighter/" + charcode + "/movie/c" + j->ToStdString() + "\""].insert("\"prebuilt:/movie/fighter/jack/c" + j->ToStdString() + "/final_00.h264\"");
						newDirFiles[i->first][j->ToStdString()]["\"fighter/" + charcode + "/movie/c" + j->ToStdString() + "\""].insert("\"prebuilt:/movie/fighter/jack/c" + j->ToStdString() + "/final_01.h264\"");

						shareToVanilla[i->first][baseSlot]["\"prebuilt:/movie/fighter/jack/c" + baseSlot + "/final_00.h264\""].insert("\"prebuilt:/movie/fighter/jack/c" + j->ToStdString() + "/final_00.h264\"");
						shareToVanilla[i->first][baseSlot]["\"prebuilt:/movie/fighter/jack/c" + baseSlot + "/final_01.h264\""].insert("\"prebuilt:/movie/fighter/jack/c" + j->ToStdString() + "/final_01.h264\"");

						// Append-case for Joker
						newDirFiles[i->first][j->ToStdString()]["\"fighter/jack/append/c" + j->ToStdString() + "\""].insert("\"fighter/jack/model/mona/c" + j->ToStdString() + "/dark_model.numatb\"");
						newDirFiles[i->first][j->ToStdString()]["\"fighter/jack/append/c" + j->ToStdString() + "\""].insert("\"fighter/jack/model/mona/c" + j->ToStdString() + "/def_mona_001_col.nutexb\"");
						newDirFiles[i->first][j->ToStdString()]["\"fighter/jack/append/c" + j->ToStdString() + "\""].insert("\"fighter/jack/model/mona/c" + j->ToStdString() + "/def_mona_001_nor.nutexb\"");
						newDirFiles[i->first][j->ToStdString()]["\"fighter/jack/append/c" + j->ToStdString() + "\""].insert("\"fighter/jack/model/mona/c" + j->ToStdString() + "/def_mona_001_prm.nutexb\"");
						newDirFiles[i->first][j->ToStdString()]["\"fighter/jack/append/c" + j->ToStdString() + "\""].insert("\"fighter/jack/model/mona/c" + j->ToStdString() + "/def_mona_002_col.nutexb\"");
						newDirFiles[i->first][j->ToStdString()]["\"fighter/jack/append/c" + j->ToStdString() + "\""].insert("\"fighter/jack/model/mona/c" + j->ToStdString() + "/def_mona_002_nor.nutexb\"");
						newDirFiles[i->first][j->ToStdString()]["\"fighter/jack/append/c" + j->ToStdString() + "\""].insert("\"fighter/jack/model/mona/c" + j->ToStdString() + "/def_mona_002_prm.nutexb\"");
						newDirFiles[i->first][j->ToStdString()]["\"fighter/jack/append/c" + j->ToStdString() + "\""].insert("\"fighter/jack/model/mona/c" + j->ToStdString() + "/eye_mona_b_l_col.nutexb\"");
						newDirFiles[i->first][j->ToStdString()]["\"fighter/jack/append/c" + j->ToStdString() + "\""].insert("\"fighter/jack/model/mona/c" + j->ToStdString() + "/eye_mona_b_r_col.nutexb\"");
						newDirFiles[i->first][j->ToStdString()]["\"fighter/jack/append/c" + j->ToStdString() + "\""].insert("\"fighter/jack/model/mona/c" + j->ToStdString() + "/eye_mona_w_col.nutexb\"");
						newDirFiles[i->first][j->ToStdString()]["\"fighter/jack/append/c" + j->ToStdString() + "\""].insert("\"fighter/jack/model/mona/c" + j->ToStdString() + "/eye_mona_w_nor.nutexb\"");
						newDirFiles[i->first][j->ToStdString()]["\"fighter/jack/append/c" + j->ToStdString() + "\""].insert("\"fighter/jack/model/mona/c" + j->ToStdString() + "/eye_mona_w_prm.nutexb\"");
						newDirFiles[i->first][j->ToStdString()]["\"fighter/jack/append/c" + j->ToStdString() + "\""].insert("\"fighter/jack/model/mona/c" + j->ToStdString() + "/light_model.numatb\"");
						newDirFiles[i->first][j->ToStdString()]["\"fighter/jack/append/c" + j->ToStdString() + "\""].insert("\"fighter/jack/model/mona/c" + j->ToStdString() + "/metamon_model.numatb\"");
						newDirFiles[i->first][j->ToStdString()]["\"fighter/jack/append/c" + j->ToStdString() + "\""].insert("\"fighter/jack/model/mona/c" + j->ToStdString() + "/model.nuhlpb\"");
						newDirFiles[i->first][j->ToStdString()]["\"fighter/jack/append/c" + j->ToStdString() + "\""].insert("\"fighter/jack/model/mona/c" + j->ToStdString() + "/model.numatb\"");
						newDirFiles[i->first][j->ToStdString()]["\"fighter/jack/append/c" + j->ToStdString() + "\""].insert("\"fighter/jack/model/mona/c" + j->ToStdString() + "/model.numdlb\"");
						newDirFiles[i->first][j->ToStdString()]["\"fighter/jack/append/c" + j->ToStdString() + "\""].insert("\"fighter/jack/model/mona/c" + j->ToStdString() + "/model.numshb\"");
						newDirFiles[i->first][j->ToStdString()]["\"fighter/jack/append/c" + j->ToStdString() + "\""].insert("\"fighter/jack/model/mona/c" + j->ToStdString() + "/model.numshexb\"");
						newDirFiles[i->first][j->ToStdString()]["\"fighter/jack/append/c" + j->ToStdString() + "\""].insert("\"fighter/jack/model/mona/c" + j->ToStdString() + "/model.nusktb\"");
						newDirFiles[i->first][j->ToStdString()]["\"fighter/jack/append/c" + j->ToStdString() + "\""].insert("\"fighter/jack/model/mona/c" + j->ToStdString() + "/model.nusrcmdlb\"");
						newDirFiles[i->first][j->ToStdString()]["\"fighter/jack/append/c" + j->ToStdString() + "\""].insert("\"fighter/jack/model/mona/c" + j->ToStdString() + "/model.xmb\"");
						newDirFiles[i->first][j->ToStdString()]["\"fighter/jack/append/c" + j->ToStdString() + "\""].insert("\"fighter/jack/model/mona/c" + j->ToStdString() + "/mona_facial_01_col.nutexb\"");
						newDirFiles[i->first][j->ToStdString()]["\"fighter/jack/append/c" + j->ToStdString() + "\""].insert("\"fighter/jack/model/mona/c" + j->ToStdString() + "/mona_facial_01_nor.nutexb\"");
						newDirFiles[i->first][j->ToStdString()]["\"fighter/jack/append/c" + j->ToStdString() + "\""].insert("\"fighter/jack/model/mona/c" + j->ToStdString() + "/mona_facial_01_prm.nutexb\"");
						newDirFiles[i->first][j->ToStdString()]["\"fighter/jack/append/c" + j->ToStdString() + "\""].insert("\"fighter/jack/model/mona/c" + j->ToStdString() + "/mona_facial_02_col.nutexb\"");
						newDirFiles[i->first][j->ToStdString()]["\"fighter/jack/append/c" + j->ToStdString() + "\""].insert("\"fighter/jack/model/mona/c" + j->ToStdString() + "/mona_facial_03_col.nutexb\"");
						newDirFiles[i->first][j->ToStdString()]["\"fighter/jack/append/c" + j->ToStdString() + "\""].insert("\"fighter/jack/model/mona/c" + j->ToStdString() + "/mona_facial_03_nor.nutexb\"");
						newDirFiles[i->first][j->ToStdString()]["\"fighter/jack/append/c" + j->ToStdString() + "\""].insert("\"fighter/jack/model/mona/c" + j->ToStdString() + "/mona_facial_03_prm.nutexb\"");
						newDirFiles[i->first][j->ToStdString()]["\"fighter/jack/append/c" + j->ToStdString() + "\""].insert("\"fighter/jack/motion/mona/c" + j->ToStdString() + "/j01appealhil.nuanmb\"");
						newDirFiles[i->first][j->ToStdString()]["\"fighter/jack/append/c" + j->ToStdString() + "\""].insert("\"fighter/jack/motion/mona/c" + j->ToStdString() + "/j01appealhir.nuanmb\"");
						newDirFiles[i->first][j->ToStdString()]["\"fighter/jack/append/c" + j->ToStdString() + "\""].insert("\"fighter/jack/motion/mona/c" + j->ToStdString() + "/j01appeallwl.nuanmb\"");
						newDirFiles[i->first][j->ToStdString()]["\"fighter/jack/append/c" + j->ToStdString() + "\""].insert("\"fighter/jack/motion/mona/c" + j->ToStdString() + "/j01appeallwr.nuanmb\"");
						newDirFiles[i->first][j->ToStdString()]["\"fighter/jack/append/c" + j->ToStdString() + "\""].insert("\"fighter/jack/motion/mona/c" + j->ToStdString() + "/j01appealsl.nuanmb\"");
						newDirFiles[i->first][j->ToStdString()]["\"fighter/jack/append/c" + j->ToStdString() + "\""].insert("\"fighter/jack/motion/mona/c" + j->ToStdString() + "/j01appealsr.nuanmb\"");
						newDirFiles[i->first][j->ToStdString()]["\"fighter/jack/append/c" + j->ToStdString() + "\""].insert("\"fighter/jack/motion/mona/c" + j->ToStdString() + "/j02win1.nuanmb\"");
						newDirFiles[i->first][j->ToStdString()]["\"fighter/jack/append/c" + j->ToStdString() + "\""].insert("\"fighter/jack/motion/mona/c" + j->ToStdString() + "/j02win3.nuanmb\"");
						newDirFiles[i->first][j->ToStdString()]["\"fighter/jack/append/c" + j->ToStdString() + "\""].insert("\"fighter/jack/motion/mona/c" + j->ToStdString() + "/motion_list.bin\"");
						newDirFiles[i->first][j->ToStdString()]["\"fighter/jack/append/c" + j->ToStdString() + "\""].insert("\"append/effect/fighter/jack_cutin/ef_jack_cutin.eff\"");
						newDirFiles[i->first][j->ToStdString()]["\"fighter/jack/append/c" + j->ToStdString() + "\""].insert("\"append/effect/fighter/jack_cutin/model/m_jackdoylecutin/c" + j->ToStdString() + "/ef_jack_cutin01.nutexb\"");
						newDirFiles[i->first][j->ToStdString()]["\"fighter/jack/append/c" + j->ToStdString() + "\""].insert("\"append/effect/fighter/jack_cutin/model/m_jackdoylecutin/c" + j->ToStdString() + "/ef_jack_cutin02.nutexb\"");
						newDirFiles[i->first][j->ToStdString()]["\"fighter/jack/append/c" + j->ToStdString() + "\""].insert("\"append/effect/fighter/jack_cutin/model/m_jackdoylecutin/c" + j->ToStdString() + "/ef_jack_cutin03.nutexb\"");
						newDirFiles[i->first][j->ToStdString()]["\"fighter/jack/append/c" + j->ToStdString() + "\""].insert("\"append/effect/fighter/jack_cutin/model/m_jackdoylecutin/c" + j->ToStdString() + "/ef_jack_cutin04.nutexb\"");
						newDirFiles[i->first][j->ToStdString()]["\"fighter/jack/append/c" + j->ToStdString() + "\""].insert("\"append/effect/fighter/jack_cutin/model/m_jackdoylecutin/c" + j->ToStdString() + "/ef_jack_cutin05.nutexb\"");
						newDirFiles[i->first][j->ToStdString()]["\"fighter/jack/append/c" + j->ToStdString() + "\""].insert("\"append/effect/fighter/jack_cutin/model/m_jackdoylecutin/c" + j->ToStdString() + "/ef_jack_cutin07.nutexb\"");
						newDirFiles[i->first][j->ToStdString()]["\"fighter/jack/append/c" + j->ToStdString() + "\""].insert("\"append/effect/fighter/jack_cutin/model/m_jackdoylecutin/c" + j->ToStdString() + "/ef_jack_cutin08.nutexb\"");
						newDirFiles[i->first][j->ToStdString()]["\"fighter/jack/append/c" + j->ToStdString() + "\""].insert("\"append/effect/fighter/jack_cutin/model/m_jackdoylecutin/c" + j->ToStdString() + "/model.nuanmb\"");
						newDirFiles[i->first][j->ToStdString()]["\"fighter/jack/append/c" + j->ToStdString() + "\""].insert("\"append/effect/fighter/jack_cutin/model/m_jackdoylecutin/c" + j->ToStdString() + "/model.nuhlpb\"");
						newDirFiles[i->first][j->ToStdString()]["\"fighter/jack/append/c" + j->ToStdString() + "\""].insert("\"append/effect/fighter/jack_cutin/model/m_jackdoylecutin/c" + j->ToStdString() + "/model.numatb\"");
						newDirFiles[i->first][j->ToStdString()]["\"fighter/jack/append/c" + j->ToStdString() + "\""].insert("\"append/effect/fighter/jack_cutin/model/m_jackdoylecutin/c" + j->ToStdString() + "/model.numdlb\"");
						newDirFiles[i->first][j->ToStdString()]["\"fighter/jack/append/c" + j->ToStdString() + "\""].insert("\"append/effect/fighter/jack_cutin/model/m_jackdoylecutin/c" + j->ToStdString() + "/model.numshb\"");
						newDirFiles[i->first][j->ToStdString()]["\"fighter/jack/append/c" + j->ToStdString() + "\""].insert("\"append/effect/fighter/jack_cutin/model/m_jackdoylecutin/c" + j->ToStdString() + "/model.numshexb\"");
						newDirFiles[i->first][j->ToStdString()]["\"fighter/jack/append/c" + j->ToStdString() + "\""].insert("\"append/effect/fighter/jack_cutin/model/m_jackdoylecutin/c" + j->ToStdString() + "/model.nusktb\"");
						newDirFiles[i->first][j->ToStdString()]["\"fighter/jack/append/c" + j->ToStdString() + "\""].insert("\"append/effect/fighter/jack_cutin/model/m_jackdoylecutin/c" + j->ToStdString() + "/model.nusrcmdlb\"");
						newDirFiles[i->first][j->ToStdString()]["\"fighter/jack/append/c" + j->ToStdString() + "\""].insert("\"append/effect/fighter/jack_cutin/model/m_jackdoylecutin/c" + j->ToStdString() + "/model.xmb\"");
						newDirFiles[i->first][j->ToStdString()]["\"fighter/jack/append/c" + j->ToStdString() + "\""].insert("\"append/effect/fighter/jack_cutin/model/m_jackdoylecutin2/c" + j->ToStdString() + "/ef_jack_character00.nutexb\"");
						newDirFiles[i->first][j->ToStdString()]["\"fighter/jack/append/c" + j->ToStdString() + "\""].insert("\"append/effect/fighter/jack_cutin/model/m_jackdoylecutin2/c" + j->ToStdString() + "/ef_jack_character01.nutexb\"");
						newDirFiles[i->first][j->ToStdString()]["\"fighter/jack/append/c" + j->ToStdString() + "\""].insert("\"append/effect/fighter/jack_cutin/model/m_jackdoylecutin2/c" + j->ToStdString() + "/ef_jack_character02.nutexb\"");
						newDirFiles[i->first][j->ToStdString()]["\"fighter/jack/append/c" + j->ToStdString() + "\""].insert("\"append/effect/fighter/jack_cutin/model/m_jackdoylecutin2/c" + j->ToStdString() + "/ef_jack_character03.nutexb\"");
						newDirFiles[i->first][j->ToStdString()]["\"fighter/jack/append/c" + j->ToStdString() + "\""].insert("\"append/effect/fighter/jack_cutin/model/m_jackdoylecutin2/c" + j->ToStdString() + "/ef_jack_character04.nutexb\"");
						newDirFiles[i->first][j->ToStdString()]["\"fighter/jack/append/c" + j->ToStdString() + "\""].insert("\"append/effect/fighter/jack_cutin/model/m_jackdoylecutin2/c" + j->ToStdString() + "/ef_jack_character05.nutexb\"");
						newDirFiles[i->first][j->ToStdString()]["\"fighter/jack/append/c" + j->ToStdString() + "\""].insert("\"append/effect/fighter/jack_cutin/model/m_jackdoylecutin2/c" + j->ToStdString() + "/ef_jack_exclamation00.nutexb\"");
						newDirFiles[i->first][j->ToStdString()]["\"fighter/jack/append/c" + j->ToStdString() + "\""].insert("\"append/effect/fighter/jack_cutin/model/m_jackdoylecutin2/c" + j->ToStdString() + "/model.nuanmb\"");
						newDirFiles[i->first][j->ToStdString()]["\"fighter/jack/append/c" + j->ToStdString() + "\""].insert("\"append/effect/fighter/jack_cutin/model/m_jackdoylecutin2/c" + j->ToStdString() + "/model.nuhlpb\"");
						newDirFiles[i->first][j->ToStdString()]["\"fighter/jack/append/c" + j->ToStdString() + "\""].insert("\"append/effect/fighter/jack_cutin/model/m_jackdoylecutin2/c" + j->ToStdString() + "/model.numatb\"");
						newDirFiles[i->first][j->ToStdString()]["\"fighter/jack/append/c" + j->ToStdString() + "\""].insert("\"append/effect/fighter/jack_cutin/model/m_jackdoylecutin2/c" + j->ToStdString() + "/model.numdlb\"");
						newDirFiles[i->first][j->ToStdString()]["\"fighter/jack/append/c" + j->ToStdString() + "\""].insert("\"append/effect/fighter/jack_cutin/model/m_jackdoylecutin2/c" + j->ToStdString() + "/model.numshb\"");
						newDirFiles[i->first][j->ToStdString()]["\"fighter/jack/append/c" + j->ToStdString() + "\""].insert("\"append/effect/fighter/jack_cutin/model/m_jackdoylecutin2/c" + j->ToStdString() + "/model.numshexb\"");
						newDirFiles[i->first][j->ToStdString()]["\"fighter/jack/append/c" + j->ToStdString() + "\""].insert("\"append/effect/fighter/jack_cutin/model/m_jackdoylecutin2/c" + j->ToStdString() + "/model.nusktb\"");
						newDirFiles[i->first][j->ToStdString()]["\"fighter/jack/append/c" + j->ToStdString() + "\""].insert("\"append/effect/fighter/jack_cutin/model/m_jackdoylecutin2/c" + j->ToStdString() + "/model.nusrcmdlb\"");
						newDirFiles[i->first][j->ToStdString()]["\"fighter/jack/append/c" + j->ToStdString() + "\""].insert("\"append/effect/fighter/jack_cutin/model/m_jackdoylecutin2/c" + j->ToStdString() + "/model.xmb\"");

						if (fighterFiles.find("fighter/jack/model/mona/c" + j->ToStdString() + "/dark_model.numatb") == fighterFiles.end())
						{
							shareToVanilla[i->first][baseSlot]["\"fighter/jack/model/mona/c" + baseSlot + "/dark_model.numatb\""].insert("\"fighter/jack/model/mona/c" + j->ToStdString() + "/dark_model.numatb\"");
						}
						if (fighterFiles.find("fighter/jack/model/mona/c" + j->ToStdString() + "/def_mona_001_col.nutexb") == fighterFiles.end())
						{
							shareToVanilla[i->first][baseSlot]["\"fighter/jack/model/mona/c" + baseSlot + "/def_mona_001_col.nutexb\""].insert("\"fighter/jack/model/mona/c" + j->ToStdString() + "/def_mona_001_col.nutexb\"");
						}
						if (fighterFiles.find("fighter/jack/model/mona/c" + j->ToStdString() + "/def_mona_001_nor.nutexb") == fighterFiles.end())
						{
							shareToVanilla[i->first][baseSlot]["\"fighter/jack/model/mona/c" + baseSlot + "/def_mona_001_nor.nutexb\""].insert("\"fighter/jack/model/mona/c" + j->ToStdString() + "/def_mona_001_nor.nutexb\"");
						}
						if (fighterFiles.find("fighter/jack/model/mona/c" + j->ToStdString() + "/def_mona_001_prm.nutexb") == fighterFiles.end())
						{
							shareToVanilla[i->first][baseSlot]["\"fighter/jack/model/mona/c" + baseSlot + "/def_mona_001_prm.nutexb\""].insert("\"fighter/jack/model/mona/c" + j->ToStdString() + "/def_mona_001_prm.nutexb\"");
						}
						if (fighterFiles.find("fighter/jack/model/mona/c" + j->ToStdString() + "/def_mona_002_col.nutexb") == fighterFiles.end())
						{
							shareToVanilla[i->first][baseSlot]["\"fighter/jack/model/mona/c" + baseSlot + "/def_mona_002_col.nutexb\""].insert("\"fighter/jack/model/mona/c" + j->ToStdString() + "/def_mona_002_col.nutexb\"");
						}
						if (fighterFiles.find("fighter/jack/model/mona/c" + j->ToStdString() + "/def_mona_002_nor.nutexb") == fighterFiles.end())
						{
							shareToVanilla[i->first][baseSlot]["\"fighter/jack/model/mona/c" + baseSlot + "/def_mona_002_nor.nutexb\""].insert("\"fighter/jack/model/mona/c" + j->ToStdString() + "/def_mona_002_nor.nutexb\"");
						}
						if (fighterFiles.find("fighter/jack/model/mona/c" + j->ToStdString() + "/def_mona_002_prm.nutexb") == fighterFiles.end())
						{
							shareToVanilla[i->first][baseSlot]["\"fighter/jack/model/mona/c" + baseSlot + "/def_mona_002_prm.nutexb\""].insert("\"fighter/jack/model/mona/c" + j->ToStdString() + "/def_mona_002_prm.nutexb\"");
						}
						if (fighterFiles.find("fighter/jack/model/mona/c" + j->ToStdString() + "/eye_mona_b_l_col.nutexb") == fighterFiles.end())
						{
							shareToVanilla[i->first][baseSlot]["\"fighter/jack/model/mona/c" + baseSlot + "/eye_mona_b_l_col.nutexb\""].insert("\"fighter/jack/model/mona/c" + j->ToStdString() + "/eye_mona_b_l_col.nutexb\"");
						}
						if (fighterFiles.find("fighter/jack/model/mona/c" + j->ToStdString() + "/eye_mona_b_r_col.nutexb") == fighterFiles.end())
						{
							shareToVanilla[i->first][baseSlot]["\"fighter/jack/model/mona/c" + baseSlot + "/eye_mona_b_r_col.nutexb\""].insert("\"fighter/jack/model/mona/c" + j->ToStdString() + "/eye_mona_b_r_col.nutexb\"");
						}
						if (fighterFiles.find("fighter/jack/model/mona/c" + j->ToStdString() + "/eye_mona_w_col.nutexb") == fighterFiles.end())
						{
							shareToVanilla[i->first][baseSlot]["\"fighter/jack/model/mona/c" + baseSlot + "/eye_mona_w_col.nutexb\""].insert("\"fighter/jack/model/mona/c" + j->ToStdString() + "/eye_mona_w_col.nutexb\"");
						}
						if (fighterFiles.find("fighter/jack/model/mona/c" + j->ToStdString() + "/eye_mona_w_nor.nutexb") == fighterFiles.end())
						{
							shareToVanilla[i->first][baseSlot]["\"fighter/jack/model/mona/c" + baseSlot + "/eye_mona_w_nor.nutexb\""].insert("\"fighter/jack/model/mona/c" + j->ToStdString() + "/eye_mona_w_nor.nutexb\"");
						}
						if (fighterFiles.find("fighter/jack/model/mona/c" + j->ToStdString() + "/eye_mona_w_prm.nutexb") == fighterFiles.end())
						{
							shareToVanilla[i->first][baseSlot]["\"fighter/jack/model/mona/c" + baseSlot + "/eye_mona_w_prm.nutexb\""].insert("\"fighter/jack/model/mona/c" + j->ToStdString() + "/eye_mona_w_prm.nutexb\"");
						}
						if (fighterFiles.find("fighter/jack/model/mona/c" + j->ToStdString() + "/light_model.numatb") == fighterFiles.end())
						{
							shareToVanilla[i->first][baseSlot]["\"fighter/jack/model/mona/c" + baseSlot + "/light_model.numatb\""].insert("\"fighter/jack/model/mona/c" + j->ToStdString() + "/light_model.numatb\"");
						}
						if (fighterFiles.find("fighter/jack/model/mona/c" + j->ToStdString() + "/metamon_model.numatb") == fighterFiles.end())
						{
							shareToVanilla[i->first][baseSlot]["\"fighter/jack/model/mona/c" + baseSlot + "/metamon_model.numatb\""].insert("\"fighter/jack/model/mona/c" + j->ToStdString() + "/metamon_model.numatb\"");
						}
						if (fighterFiles.find("fighter/jack/model/mona/c" + j->ToStdString() + "/model.nuhlpb") == fighterFiles.end())
						{
							shareToVanilla[i->first][baseSlot]["\"fighter/jack/model/mona/c" + baseSlot + "/model.nuhlpb\""].insert("\"fighter/jack/model/mona/c" + j->ToStdString() + "/model.nuhlpb\"");
						}
						if (fighterFiles.find("fighter/jack/model/mona/c" + j->ToStdString() + "/model.numatb") == fighterFiles.end())
						{
							shareToVanilla[i->first][baseSlot]["\"fighter/jack/model/mona/c" + baseSlot + "/model.numatb\""].insert("\"fighter/jack/model/mona/c" + j->ToStdString() + "/model.numatb\"");
						}
						if (fighterFiles.find("fighter/jack/model/mona/c" + j->ToStdString() + "/model.numdlb") == fighterFiles.end())
						{
							shareToVanilla[i->first][baseSlot]["\"fighter/jack/model/mona/c" + baseSlot + "/model.numdlb\""].insert("\"fighter/jack/model/mona/c" + j->ToStdString() + "/model.numdlb\"");
						}
						if (fighterFiles.find("fighter/jack/model/mona/c" + j->ToStdString() + "/model.numshb") == fighterFiles.end())
						{
							shareToVanilla[i->first][baseSlot]["\"fighter/jack/model/mona/c" + baseSlot + "/model.numshb\""].insert("\"fighter/jack/model/mona/c" + j->ToStdString() + "/model.numshb\"");
						}
						if (fighterFiles.find("fighter/jack/model/mona/c" + j->ToStdString() + "/model.numshexb") == fighterFiles.end())
						{
							shareToVanilla[i->first][baseSlot]["\"fighter/jack/model/mona/c" + baseSlot + "/model.numshexb\""].insert("\"fighter/jack/model/mona/c" + j->ToStdString() + "/model.numshexb\"");
						}
						if (fighterFiles.find("fighter/jack/model/mona/c" + j->ToStdString() + "/model.nusktb") == fighterFiles.end())
						{
							shareToVanilla[i->first][baseSlot]["\"fighter/jack/model/mona/c" + baseSlot + "/model.nusktb\""].insert("\"fighter/jack/model/mona/c" + j->ToStdString() + "/model.nusktb\"");
						}
						if (fighterFiles.find("fighter/jack/model/mona/c" + j->ToStdString() + "/model.nusrcmdlb") == fighterFiles.end())
						{
							shareToVanilla[i->first][baseSlot]["\"fighter/jack/model/mona/c" + baseSlot + "/model.nusrcmdlb\""].insert("\"fighter/jack/model/mona/c" + j->ToStdString() + "/model.nusrcmdlb\"");
						}
						if (fighterFiles.find("fighter/jack/model/mona/c" + j->ToStdString() + "/model.xmb") == fighterFiles.end())
						{
							shareToVanilla[i->first][baseSlot]["\"fighter/jack/model/mona/c" + baseSlot + "/model.xmb\""].insert("\"fighter/jack/model/mona/c" + j->ToStdString() + "/model.xmb\"");
						}
						if (fighterFiles.find("fighter/jack/model/mona/c" + j->ToStdString() + "/mona_facial_01_col.nutexb") == fighterFiles.end())
						{
							shareToVanilla[i->first][baseSlot]["\"fighter/jack/model/mona/c" + baseSlot + "/mona_facial_01_col.nutexb\""].insert("\"fighter/jack/model/mona/c" + j->ToStdString() + "/mona_facial_01_col.nutexb\"");
						}
						if (fighterFiles.find("fighter/jack/model/mona/c" + j->ToStdString() + "/mona_facial_01_nor.nutexb") == fighterFiles.end())
						{
							shareToVanilla[i->first][baseSlot]["\"fighter/jack/model/mona/c" + baseSlot + "/mona_facial_01_nor.nutexb\""].insert("\"fighter/jack/model/mona/c" + j->ToStdString() + "/mona_facial_01_nor.nutexb\"");
						}
						if (fighterFiles.find("fighter/jack/model/mona/c" + j->ToStdString() + "/mona_facial_01_prm.nutexb") == fighterFiles.end())
						{
							shareToVanilla[i->first][baseSlot]["\"fighter/jack/model/mona/c" + baseSlot + "/mona_facial_01_prm.nutexb\""].insert("\"fighter/jack/model/mona/c" + j->ToStdString() + "/mona_facial_01_prm.nutexb\"");
						}
						if (fighterFiles.find("fighter/jack/model/mona/c" + j->ToStdString() + "/mona_facial_02_col.nutexb") == fighterFiles.end())
						{
							shareToVanilla[i->first][baseSlot]["\"fighter/jack/model/mona/c" + baseSlot + "/mona_facial_02_col.nutexb\""].insert("\"fighter/jack/model/mona/c" + j->ToStdString() + "/mona_facial_02_col.nutexb\"");
						}
						if (fighterFiles.find("fighter/jack/model/mona/c" + j->ToStdString() + "/mona_facial_03_col.nutexb") == fighterFiles.end())
						{
							shareToVanilla[i->first][baseSlot]["\"fighter/jack/model/mona/c" + baseSlot + "/mona_facial_03_col.nutexb\""].insert("\"fighter/jack/model/mona/c" + j->ToStdString() + "/mona_facial_03_col.nutexb\"");
						}
						if (fighterFiles.find("fighter/jack/model/mona/c" + j->ToStdString() + "/mona_facial_03_nor.nutexb") == fighterFiles.end())
						{
							shareToVanilla[i->first][baseSlot]["\"fighter/jack/model/mona/c" + baseSlot + "/mona_facial_03_nor.nutexb\""].insert("\"fighter/jack/model/mona/c" + j->ToStdString() + "/mona_facial_03_nor.nutexb\"");
						}
						if (fighterFiles.find("fighter/jack/model/mona/c" + j->ToStdString() + "/mona_facial_03_prm.nutexb") == fighterFiles.end())
						{
							shareToVanilla[i->first][baseSlot]["\"fighter/jack/model/mona/c" + baseSlot + "/mona_facial_03_prm.nutexb\""].insert("\"fighter/jack/model/mona/c" + j->ToStdString() + "/mona_facial_03_prm.nutexb\"");
						}
						if (fighterFiles.find("fighter/jack/motion/mona/c" + j->ToStdString() + "/j01appealhil.nuanmb") == fighterFiles.end())
						{
							shareToAdded[i->first][baseSlot]["\"fighter/jack/motion/mona/c" + baseSlot + "/j01appealhil.nuanmb\""].insert("\"fighter/jack/motion/mona/c" + j->ToStdString() + "/j01appealhil.nuanmb\"");
						}
						if (fighterFiles.find("fighter/jack/motion/mona/c" + j->ToStdString() + "/j01appealhir.nuanmb") == fighterFiles.end())
						{
							shareToAdded[i->first][baseSlot]["\"fighter/jack/motion/mona/c" + baseSlot + "/j01appealhir.nuanmb\""].insert("\"fighter/jack/motion/mona/c" + j->ToStdString() + "/j01appealhir.nuanmb\"");
						}
						if (fighterFiles.find("fighter/jack/motion/mona/c" + j->ToStdString() + "/j01appeallwl.nuanmb") == fighterFiles.end())
						{
							shareToAdded[i->first][baseSlot]["\"fighter/jack/motion/mona/c" + baseSlot + "/j01appeallwl.nuanmb\""].insert("\"fighter/jack/motion/mona/c" + j->ToStdString() + "/j01appeallwl.nuanmb\"");
						}
						if (fighterFiles.find("fighter/jack/motion/mona/c" + j->ToStdString() + "/j01appeallwr.nuanmb") == fighterFiles.end())
						{
							shareToAdded[i->first][baseSlot]["\"fighter/jack/motion/mona/c" + baseSlot + "/j01appeallwr.nuanmb\""].insert("\"fighter/jack/motion/mona/c" + j->ToStdString() + "/j01appeallwr.nuanmb\"");
						}
						if (fighterFiles.find("fighter/jack/motion/mona/c" + j->ToStdString() + "/j01appealsl.nuanmb") == fighterFiles.end())
						{
							shareToAdded[i->first][baseSlot]["\"fighter/jack/motion/mona/c" + baseSlot + "/j01appealsl.nuanmb\""].insert("\"fighter/jack/motion/mona/c" + j->ToStdString() + "/j01appealsl.nuanmb\"");
						}
						if (fighterFiles.find("fighter/jack/motion/mona/c" + j->ToStdString() + "/j01appealsr.nuanmb") == fighterFiles.end())
						{
							shareToAdded[i->first][baseSlot]["\"fighter/jack/motion/mona/c" + baseSlot + "/j01appealsr.nuanmb\""].insert("\"fighter/jack/motion/mona/c" + j->ToStdString() + "/j01appealsr.nuanmb\"");
						}
						if (fighterFiles.find("fighter/jack/motion/mona/c" + j->ToStdString() + "/j02win1.nuanmb") == fighterFiles.end())
						{
							shareToAdded[i->first][baseSlot]["\"fighter/jack/motion/mona/c" + baseSlot + "/j02win1.nuanmb\""].insert("\"fighter/jack/motion/mona/c" + j->ToStdString() + "/j02win1.nuanmb\"");
						}
						if (fighterFiles.find("fighter/jack/motion/mona/c" + j->ToStdString() + "/j02win3.nuanmb") == fighterFiles.end())
						{
							shareToAdded[i->first][baseSlot]["\"fighter/jack/motion/mona/c" + baseSlot + "/j02win3.nuanmb\""].insert("\"fighter/jack/motion/mona/c" + j->ToStdString() + "/j02win3.nuanmb\"");
						}
						if (fighterFiles.find("fighter/jack/motion/mona/c" + j->ToStdString() + "/motion_list.bin") == fighterFiles.end())
						{
							shareToAdded[i->first][baseSlot]["\"fighter/jack/motion/mona/c" + baseSlot + "/motion_list.bin\""].insert("\"fighter/jack/motion/mona/c" + j->ToStdString() + "/motion_list.bin\"");
						}
						if (fighterFiles.find("append/effect/fighter/jack_cutin/ef_jack_cutin.eff") == fighterFiles.end())
						{
							shareToVanilla[i->first][baseSlot]["\"append/effect/fighter/jack_cutin/ef_jack_cutin.eff\""].insert("\"append/effect/fighter/jack_cutin/ef_jack_cutin.eff\"");
						}
						if (fighterFiles.find("append/effect/fighter/jack_cutin/model/m_jackdoylecutin/c" + j->ToStdString() + "/ef_jack_cutin01.nutexb") == fighterFiles.end())
						{
							shareToVanilla[i->first][baseSlot]["\"append/effect/fighter/jack_cutin/model/m_jackdoylecutin/c" + baseSlot + "/ef_jack_cutin01.nutexb\""].insert("\"append/effect/fighter/jack_cutin/model/m_jackdoylecutin/c" + j->ToStdString() + "/ef_jack_cutin01.nutexb\"");
						}
						if (fighterFiles.find("append/effect/fighter/jack_cutin/model/m_jackdoylecutin/c" + j->ToStdString() + "/ef_jack_cutin02.nutexb") == fighterFiles.end())
						{
							shareToVanilla[i->first][baseSlot]["\"append/effect/fighter/jack_cutin/model/m_jackdoylecutin/c" + baseSlot + "/ef_jack_cutin02.nutexb\""].insert("\"append/effect/fighter/jack_cutin/model/m_jackdoylecutin/c" + j->ToStdString() + "/ef_jack_cutin02.nutexb\"");
						}
						if (fighterFiles.find("append/effect/fighter/jack_cutin/model/m_jackdoylecutin/c" + j->ToStdString() + "/ef_jack_cutin03.nutexb") == fighterFiles.end())
						{
							shareToVanilla[i->first][baseSlot]["\"append/effect/fighter/jack_cutin/model/m_jackdoylecutin/c" + baseSlot + "/ef_jack_cutin03.nutexb\""].insert("\"append/effect/fighter/jack_cutin/model/m_jackdoylecutin/c" + j->ToStdString() + "/ef_jack_cutin03.nutexb\"");
						}
						if (fighterFiles.find("append/effect/fighter/jack_cutin/model/m_jackdoylecutin/c" + j->ToStdString() + "/ef_jack_cutin04.nutexb") == fighterFiles.end())
						{
							shareToVanilla[i->first][baseSlot]["\"append/effect/fighter/jack_cutin/model/m_jackdoylecutin/c" + baseSlot + "/ef_jack_cutin04.nutexb\""].insert("\"append/effect/fighter/jack_cutin/model/m_jackdoylecutin/c" + j->ToStdString() + "/ef_jack_cutin04.nutexb\"");
						}
						if (fighterFiles.find("append/effect/fighter/jack_cutin/model/m_jackdoylecutin/c" + j->ToStdString() + "/ef_jack_cutin05.nutexb") == fighterFiles.end())
						{
							shareToVanilla[i->first][baseSlot]["\"append/effect/fighter/jack_cutin/model/m_jackdoylecutin/c" + baseSlot + "/ef_jack_cutin05.nutexb\""].insert("\"append/effect/fighter/jack_cutin/model/m_jackdoylecutin/c" + j->ToStdString() + "/ef_jack_cutin05.nutexb\"");
						}
						if (fighterFiles.find("append/effect/fighter/jack_cutin/model/m_jackdoylecutin/c" + j->ToStdString() + "/ef_jack_cutin07.nutexb") == fighterFiles.end())
						{
							shareToVanilla[i->first][baseSlot]["\"append/effect/fighter/jack_cutin/model/m_jackdoylecutin/c" + baseSlot + "/ef_jack_cutin07.nutexb\""].insert("\"append/effect/fighter/jack_cutin/model/m_jackdoylecutin/c" + j->ToStdString() + "/ef_jack_cutin07.nutexb\"");
						}
						if (fighterFiles.find("append/effect/fighter/jack_cutin/model/m_jackdoylecutin/c" + j->ToStdString() + "/ef_jack_cutin08.nutexb") == fighterFiles.end())
						{
							shareToVanilla[i->first][baseSlot]["\"append/effect/fighter/jack_cutin/model/m_jackdoylecutin/c" + baseSlot + "/ef_jack_cutin08.nutexb\""].insert("\"append/effect/fighter/jack_cutin/model/m_jackdoylecutin/c" + j->ToStdString() + "/ef_jack_cutin08.nutexb\"");
						}
						if (fighterFiles.find("append/effect/fighter/jack_cutin/model/m_jackdoylecutin/c" + j->ToStdString() + "/model.nuanmb") == fighterFiles.end())
						{
							shareToVanilla[i->first][baseSlot]["\"append/effect/fighter/jack_cutin/model/m_jackdoylecutin/c" + baseSlot + "/model.nuanmb\""].insert("\"append/effect/fighter/jack_cutin/model/m_jackdoylecutin/c" + j->ToStdString() + "/model.nuanmb\"");
						}
						if (fighterFiles.find("append/effect/fighter/jack_cutin/model/m_jackdoylecutin/c" + j->ToStdString() + "/model.nuhlpb") == fighterFiles.end())
						{
							shareToVanilla[i->first][baseSlot]["\"append/effect/fighter/jack_cutin/model/m_jackdoylecutin/c" + baseSlot + "/model.nuhlpb\""].insert("\"append/effect/fighter/jack_cutin/model/m_jackdoylecutin/c" + j->ToStdString() + "/model.nuhlpb\"");
						}
						if (fighterFiles.find("append/effect/fighter/jack_cutin/model/m_jackdoylecutin/c" + j->ToStdString() + "/model.numatb") == fighterFiles.end())
						{
							shareToVanilla[i->first][baseSlot]["\"append/effect/fighter/jack_cutin/model/m_jackdoylecutin/c" + baseSlot + "/model.numatb\""].insert("\"append/effect/fighter/jack_cutin/model/m_jackdoylecutin/c" + j->ToStdString() + "/model.numatb\"");
						}
						if (fighterFiles.find("append/effect/fighter/jack_cutin/model/m_jackdoylecutin/c" + j->ToStdString() + "/model.numdlb") == fighterFiles.end())
						{
							shareToVanilla[i->first][baseSlot]["\"append/effect/fighter/jack_cutin/model/m_jackdoylecutin/c" + baseSlot + "/model.numdlb\""].insert("\"append/effect/fighter/jack_cutin/model/m_jackdoylecutin/c" + j->ToStdString() + "/model.numdlb\"");
						}
						if (fighterFiles.find("append/effect/fighter/jack_cutin/model/m_jackdoylecutin/c" + j->ToStdString() + "/model.numshb") == fighterFiles.end())
						{
							shareToVanilla[i->first][baseSlot]["\"append/effect/fighter/jack_cutin/model/m_jackdoylecutin/c" + baseSlot + "/model.numshb\""].insert("\"append/effect/fighter/jack_cutin/model/m_jackdoylecutin/c" + j->ToStdString() + "/model.numshb\"");
						}
						if (fighterFiles.find("append/effect/fighter/jack_cutin/model/m_jackdoylecutin/c" + j->ToStdString() + "/model.numshexb") == fighterFiles.end())
						{
							shareToVanilla[i->first][baseSlot]["\"append/effect/fighter/jack_cutin/model/m_jackdoylecutin/c" + baseSlot + "/model.numshexb\""].insert("\"append/effect/fighter/jack_cutin/model/m_jackdoylecutin/c" + j->ToStdString() + "/model.numshexb\"");
						}
						if (fighterFiles.find("append/effect/fighter/jack_cutin/model/m_jackdoylecutin/c" + j->ToStdString() + "/model.nusktb") == fighterFiles.end())
						{
							shareToVanilla[i->first][baseSlot]["\"append/effect/fighter/jack_cutin/model/m_jackdoylecutin/c" + baseSlot + "/model.nusktb\""].insert("\"append/effect/fighter/jack_cutin/model/m_jackdoylecutin/c" + j->ToStdString() + "/model.nusktb\"");
						}
						if (fighterFiles.find("append/effect/fighter/jack_cutin/model/m_jackdoylecutin/c" + j->ToStdString() + "/model.nusrcmdlb") == fighterFiles.end())
						{
							shareToVanilla[i->first][baseSlot]["\"append/effect/fighter/jack_cutin/model/m_jackdoylecutin/c" + baseSlot + "/model.nusrcmdlb\""].insert("\"append/effect/fighter/jack_cutin/model/m_jackdoylecutin/c" + j->ToStdString() + "/model.nusrcmdlb\"");
						}
						if (fighterFiles.find("append/effect/fighter/jack_cutin/model/m_jackdoylecutin/c" + j->ToStdString() + "/model.xmb") == fighterFiles.end())
						{
							shareToVanilla[i->first][baseSlot]["\"append/effect/fighter/jack_cutin/model/m_jackdoylecutin/c" + baseSlot + "/model.xmb\""].insert("\"append/effect/fighter/jack_cutin/model/m_jackdoylecutin/c" + j->ToStdString() + "/model.xmb\"");
						}
						if (fighterFiles.find("append/effect/fighter/jack_cutin/model/m_jackdoylecutin2/c" + j->ToStdString() + "/ef_jack_character00.nutexb") == fighterFiles.end())
						{
							shareToVanilla[i->first][baseSlot]["\"append/effect/fighter/jack_cutin/model/m_jackdoylecutin2/c" + baseSlot + "/ef_jack_character00.nutexb\""].insert("\"append/effect/fighter/jack_cutin/model/m_jackdoylecutin2/c" + j->ToStdString() + "/ef_jack_character00.nutexb\"");
						}
						if (fighterFiles.find("append/effect/fighter/jack_cutin/model/m_jackdoylecutin2/c" + j->ToStdString() + "/ef_jack_character01.nutexb") == fighterFiles.end())
						{
							shareToVanilla[i->first][baseSlot]["\"append/effect/fighter/jack_cutin/model/m_jackdoylecutin2/c" + baseSlot + "/ef_jack_character01.nutexb\""].insert("\"append/effect/fighter/jack_cutin/model/m_jackdoylecutin2/c" + j->ToStdString() + "/ef_jack_character01.nutexb\"");
						}
						if (fighterFiles.find("append/effect/fighter/jack_cutin/model/m_jackdoylecutin2/c" + j->ToStdString() + "/ef_jack_character02.nutexb") == fighterFiles.end())
						{
							shareToVanilla[i->first][baseSlot]["\"append/effect/fighter/jack_cutin/model/m_jackdoylecutin2/c" + baseSlot + "/ef_jack_character02.nutexb\""].insert("\"append/effect/fighter/jack_cutin/model/m_jackdoylecutin2/c" + j->ToStdString() + "/ef_jack_character02.nutexb\"");
						}
						if (fighterFiles.find("append/effect/fighter/jack_cutin/model/m_jackdoylecutin2/c" + j->ToStdString() + "/ef_jack_character03.nutexb") == fighterFiles.end())
						{
							shareToVanilla[i->first][baseSlot]["\"append/effect/fighter/jack_cutin/model/m_jackdoylecutin2/c" + baseSlot + "/ef_jack_character03.nutexb\""].insert("\"append/effect/fighter/jack_cutin/model/m_jackdoylecutin2/c" + j->ToStdString() + "/ef_jack_character03.nutexb\"");
						}
						if (fighterFiles.find("append/effect/fighter/jack_cutin/model/m_jackdoylecutin2/c" + j->ToStdString() + "/ef_jack_character04.nutexb") == fighterFiles.end())
						{
							shareToVanilla[i->first][baseSlot]["\"append/effect/fighter/jack_cutin/model/m_jackdoylecutin2/c" + baseSlot + "/ef_jack_character04.nutexb\""].insert("\"append/effect/fighter/jack_cutin/model/m_jackdoylecutin2/c" + j->ToStdString() + "/ef_jack_character04.nutexb\"");
						}
						if (fighterFiles.find("append/effect/fighter/jack_cutin/model/m_jackdoylecutin2/c" + j->ToStdString() + "/ef_jack_character05.nutexb") == fighterFiles.end())
						{
							shareToVanilla[i->first][baseSlot]["\"append/effect/fighter/jack_cutin/model/m_jackdoylecutin2/c" + baseSlot + "/ef_jack_character05.nutexb\""].insert("\"append/effect/fighter/jack_cutin/model/m_jackdoylecutin2/c" + j->ToStdString() + "/ef_jack_character05.nutexb\"");
						}
						if (fighterFiles.find("append/effect/fighter/jack_cutin/model/m_jackdoylecutin2/c" + j->ToStdString() + "/ef_jack_exclamation00.nutexb") == fighterFiles.end())
						{
							shareToVanilla[i->first][baseSlot]["\"append/effect/fighter/jack_cutin/model/m_jackdoylecutin2/c" + baseSlot + "/ef_jack_exclamation00.nutexb\""].insert("\"append/effect/fighter/jack_cutin/model/m_jackdoylecutin2/c" + j->ToStdString() + "/ef_jack_exclamation00.nutexb\"");
						}
						if (fighterFiles.find("append/effect/fighter/jack_cutin/model/m_jackdoylecutin2/c" + j->ToStdString() + "/model.nuanmb") == fighterFiles.end())
						{
							shareToVanilla[i->first][baseSlot]["\"append/effect/fighter/jack_cutin/model/m_jackdoylecutin2/c" + baseSlot + "/model.nuanmb\""].insert("\"append/effect/fighter/jack_cutin/model/m_jackdoylecutin2/c" + j->ToStdString() + "/model.nuanmb\"");
						}
						if (fighterFiles.find("append/effect/fighter/jack_cutin/model/m_jackdoylecutin2/c" + j->ToStdString() + "/model.nuhlpb") == fighterFiles.end())
						{
							shareToVanilla[i->first][baseSlot]["\"append/effect/fighter/jack_cutin/model/m_jackdoylecutin2/c" + baseSlot + "/model.nuhlpb\""].insert("\"append/effect/fighter/jack_cutin/model/m_jackdoylecutin2/c" + j->ToStdString() + "/model.nuhlpb\"");
						}
						if (fighterFiles.find("append/effect/fighter/jack_cutin/model/m_jackdoylecutin2/c" + j->ToStdString() + "/model.numatb") == fighterFiles.end())
						{
							shareToVanilla[i->first][baseSlot]["\"append/effect/fighter/jack_cutin/model/m_jackdoylecutin2/c" + baseSlot + "/model.numatb\""].insert("\"append/effect/fighter/jack_cutin/model/m_jackdoylecutin2/c" + j->ToStdString() + "/model.numatb\"");
						}
						if (fighterFiles.find("append/effect/fighter/jack_cutin/model/m_jackdoylecutin2/c" + j->ToStdString() + "/model.numdlb") == fighterFiles.end())
						{
							shareToVanilla[i->first][baseSlot]["\"append/effect/fighter/jack_cutin/model/m_jackdoylecutin2/c" + baseSlot + "/model.numdlb\""].insert("\"append/effect/fighter/jack_cutin/model/m_jackdoylecutin2/c" + j->ToStdString() + "/model.numdlb\"");
						}
						if (fighterFiles.find("append/effect/fighter/jack_cutin/model/m_jackdoylecutin2/c" + j->ToStdString() + "/model.numshb") == fighterFiles.end())
						{
							shareToVanilla[i->first][baseSlot]["\"append/effect/fighter/jack_cutin/model/m_jackdoylecutin2/c" + baseSlot + "/model.numshb\""].insert("\"append/effect/fighter/jack_cutin/model/m_jackdoylecutin2/c" + j->ToStdString() + "/model.numshb\"");
						}
						if (fighterFiles.find("append/effect/fighter/jack_cutin/model/m_jackdoylecutin2/c" + j->ToStdString() + "/model.numshexb") == fighterFiles.end())
						{
							shareToVanilla[i->first][baseSlot]["\"append/effect/fighter/jack_cutin/model/m_jackdoylecutin2/c" + baseSlot + "/model.numshexb\""].insert("\"append/effect/fighter/jack_cutin/model/m_jackdoylecutin2/c" + j->ToStdString() + "/model.numshexb\"");
						}
						if (fighterFiles.find("append/effect/fighter/jack_cutin/model/m_jackdoylecutin2/c" + j->ToStdString() + "/model.nusktb") == fighterFiles.end())
						{
							shareToVanilla[i->first][baseSlot]["\"append/effect/fighter/jack_cutin/model/m_jackdoylecutin2/c" + baseSlot + "/model.nusktb\""].insert("\"append/effect/fighter/jack_cutin/model/m_jackdoylecutin2/c" + j->ToStdString() + "/model.nusktb\"");
						}
						if (fighterFiles.find("append/effect/fighter/jack_cutin/model/m_jackdoylecutin2/c" + j->ToStdString() + "/model.nusrcmdlb") == fighterFiles.end())
						{
							shareToVanilla[i->first][baseSlot]["\"append/effect/fighter/jack_cutin/model/m_jackdoylecutin2/c" + baseSlot + "/model.nusrcmdlb\""].insert("\"append/effect/fighter/jack_cutin/model/m_jackdoylecutin2/c" + j->ToStdString() + "/model.nusrcmdlb\"");
						}
						if (fighterFiles.find("append/effect/fighter/jack_cutin/model/m_jackdoylecutin2/c" + j->ToStdString() + "/model.xmb") == fighterFiles.end())
						{
							shareToVanilla[i->first][baseSlot]["\"append/effect/fighter/jack_cutin/model/m_jackdoylecutin2/c" + baseSlot + "/model.xmb\""].insert("\"append/effect/fighter/jack_cutin/model/m_jackdoylecutin2/c" + j->ToStdString() + "/model.xmb\"");
						}
					}
					else
					{
						newDirFiles[i->first][j->ToStdString()]["\"fighter/" + charcode + "/movie/c" + j->ToStdString() + "\""];
					}

					// Add result files.
					if (newDirFiles[i->first][j->ToStdString()].find("\"fighter/" + charcode + "/result/c" + j->ToStdString() + "\"") == newDirFiles[i->first][j->ToStdString()].end())
					{
						newDirFiles[i->first][j->ToStdString()]["\"fighter/" + charcode + "/result/c" + j->ToStdString() + "\""];
					}
				}
			}
			
			// Add NEW fighter files to newDirFiles
			if (slotHasFighter)
			{
				for (auto k = fighterFiles.begin(); k != fighterFiles.end(); k++)
				{
					string basePath;

					// Additional Slot: Convert path to base slot's path
					if (additionalSlot)
					{
						std::size_t cPos = k->find("/c" + *j);

						if (cPos != string::npos)
						{
							basePath = *k;
							basePath.replace(cPos, 2 + j->size(), "/c" + baseSlot);
						}
					}

					if (additionalSlot)
					{
						if (vanillaFiles[charcode][baseSlot].find(basePath) == vanillaFiles[charcode][baseSlot].end())
						{
							if (vanillaKirbyFiles[charcode][baseSlot].find(basePath) == vanillaKirbyFiles[charcode][baseSlot].end())
							{
								newDirFiles[i->first][j->ToStdString()]["\"fighter/" + charcode + "/c" + j->ToStdString() + "\""].insert("\"" + *k + "\"");
							}
						}
					}
					else if (vanillaFiles[charcode][j->ToStdString()].find(*k) == vanillaFiles[charcode][j->ToStdString()].end())
					{
						bool found = false;

						if (charcode == "kirby")
						{
							size_t copyPos = k->find("copy_");
							if (copyPos != string::npos)
							{
								// fighter/kirby/[motion/body]/copy_[charcode]_[...]/c[slot]/...
								string charCopy = k->substr(copyPos + 5, k->find("_", copyPos + 5) - copyPos - 5);
								size_t slotPos = k->find("/c", copyPos);

								if (slotPos != string::npos)
								{
									string charSlot = k->substr(slotPos + 2, k->find('/', slotPos + 1) - slotPos - 2);

									if (vanillaKirbyFiles[charCopy][charSlot].find(*k) != vanillaKirbyFiles[charCopy][charSlot].end())
									{
										found = true;
									}
								}
							}
						}

						if (!found)
						{
							newDirFiles[i->first][j->ToStdString()]["\"fighter/" + charcode + "/c" + j->ToStdString() + "\""].insert("\"" + *k + "\"");
						}
					}
				}
			}

			// One Slot Files detected
			if (slotHasEffect)
			{
				// Add MISSING effect files to shareToVanilla & newDirFiles
				for (auto k = vanillaEffectFiles[charcode].begin(); k != vanillaEffectFiles[charcode].end(); k++)
				{
					string path = *k;

					// eff file
					// [...].eff = 4
					if (path.substr(path.size() - 4, 4) == ".eff")
					{
						path = path.substr(0, path.size() - 4) + "_c" + *j + ".eff";
					}
					else
					{
						// model folder
						if (path.find("/model/") != string::npos)
						{
							// /model/ = 7
							std::size_t slashPos = path.find('/', path.find("/model/") + 7);
							path = path.substr(0, slashPos) + "_c" + *j + path.substr(slashPos);
						}
						else
						{
							// effect/fighter/[charcode]/[folder type]/...
							// [charcode]/[folder type]"/"...
							// slashPos = slash after charcode's slash
							std::size_t slashPos = path.find('/', path.find(charcode) + charcode.size() + 1);
							path = path.substr(0, slashPos) + "_c" + *j + path.substr(slashPos);
						}
					}

					if (effectFiles.find(path) == effectFiles.end())
					{
						shareToVanilla[i->first]["all"]["\"" + *k + "\""].insert("\"" + path + "\"");
						newDirFiles[i->first][j->ToStdString()]["\"fighter/" + charcode + "/c" + j->ToStdString() + "\""].insert("\"" + path + "\"");
					}
				}

				// Add NEW effect files to newDirFiles
				for (auto k = effectFiles.begin(); k != effectFiles.end(); k++)
				{
					if (vanillaEffectFiles[charcode].find(*k) == vanillaEffectFiles[charcode].end())
					{
						newDirFiles[i->first][j->ToStdString()]["\"fighter/" + charcode + "/c" + j->ToStdString() + "\""].insert("\"" + *k + "\"");
					}
				}
			}
		}

		if (!hasElement && charcode == "eflame")
		{
			charcode = "element";
		}
		else if(charcode != "popo")
		{
			i++;
		}
	}
}

void SmashData::createConfig(map<string, map<string, set<string>>>& baseSlots)
{
	vector<string> newDirInfos;
	vector<string> newDirInfosBase;
	map<string, map<string, map<string, set<string>>>> shareToVanilla;
	map<string, map<string, map<string, set<string>>>> shareToAdded;
	map<string, map<string, map<string, set<string>>>> newDirFiles;

	this->getNewDirSlots(baseSlots, newDirInfos, newDirInfosBase, shareToVanilla, shareToAdded, newDirFiles);

	if (!newDirInfos.empty() || !newDirInfosBase.empty() || !shareToVanilla.empty() || !shareToAdded.empty() || !newDirFiles.empty())
	{
		bool first = true;

		ofstream configFile;
		configFile.open(rootPath + "/" + "config.json", std::ios::out | std::ios::trunc);

		if (configFile.is_open())
		{
			configFile << "{";

			if (!newDirInfos.empty())
			{
				configFile << "\n\t\"new-dir-infos\": [";

				for (auto i = newDirInfos.begin(); i != newDirInfos.end(); i++)
				{
					if (first)
					{
						first = false;
					}
					else
					{
						configFile << ",";
					}

					configFile << "\n\t\t" << *i;
				}

				configFile << "\n\t]";
			}

			if (!newDirInfosBase.empty())
			{
				if (!first)
				{
					configFile << ",";
					first = true;
				}

				configFile << "\n\t\"new-dir-infos-base\": {";

				for (auto i = newDirInfosBase.begin(); i != newDirInfosBase.end(); i++)
				{
					if (first)
					{
						first = false;
					}
					else
					{
						configFile << ",";
					}

					configFile << "\n\t\t" << *i;
				}

				configFile << "\n\t}";
			}

			vector sectionType = { &shareToVanilla, &shareToAdded, &newDirFiles };
			for (auto i = 0; i < sectionType.size(); i++)
			{
				if (!((*sectionType[i]).empty()))
				{
					if (!first)
					{
						configFile << ",";
						first = true;
					}

					if (i == 0)
					{
						configFile << "\n\t\"share-to-vanilla\": {";
					}
					else if (i == 1)
					{
						configFile << "\n\t\"share-to-added\": {";
					}
					else
					{
						configFile << "\n\t\"new-dir-files\": {";
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
										configFile << ",";
									}

									if (firstBox)
									{
										configFile << "\n\t\t" + l->first << ": [";
										firstBox = false;
									}

									configFile << "\n\t\t\t" << *m;
								}

								if (!firstBox)
								{
									configFile << "\n\t\t]";
								}
								else
								{
									if (first)
									{
										first = false;
									}
									else
									{
										configFile << ",";
									}

									configFile << "\n\t\t" + l->first << ": []";
								}
							}
						}
					}

					configFile << "\n\t}";
				}
			}

			configFile << "\n}";
			configFile.close();

			log->LogText("> Success: Config file was created!");
		}
		else
		{
			log->LogText("> Error: " + (rootPath + "/" + "config.json") + " could not be opened!");
		}
	}
	else
	{
		log->LogText("> N/A: Config file is not needed!");
	}
}

void SmashData::patchXMLSlots(map<string, int>& maxSlots)
{
	if (!maxSlots.empty())
	{
		ifstream input("ui_chara_db.xml");
		ofstream output("ui_chara_db_EDIT.xml");

		auto additionalSlots = getAdditionalSlots();
		vector<int> defaultCs;

		if (input.is_open() && output.is_open())
		{
			string charcode;
			string line;

			bool changeActive = false;

			while (!input.eof())
			{
				getline(input, line);

				if (changeActive)
				{
					if (line.find("\"color_num\"") != string::npos)
					{
						line = "      <byte hash=\"color_num\">" + to_string(maxSlots[charcode]) + "</byte>";
					}
					else if (line.find("<byte hash=\"c0") != string::npos)
					{
						defaultCs.push_back(line[line.find('>') + 1]);

						// Last c reached, start adding new ones
						if (line.find("c07") != string::npos)
						{
							output << line << "\n";

							string lineBeg = "      <byte hash=\"c";
							string lineMid;
							if (line.find("c07_group") != string::npos)
							{
								lineMid = "_group\">";
							}
							else
							{
								lineMid = "_index\">";
							}
							string lineEnd = "</byte>";

							for (auto i = additionalSlots[charcode].begin(); i != additionalSlots[charcode].end(); i++)
							{
								char val;

								// Find base slot
								for (auto j = baseSlots[charcode].begin(); j != baseSlots[charcode].end(); j++)
								{
									if (j->second.find(*i) != j->second.end())
									{
										val = defaultCs[stoi(j->first)];
										break;
									}
								}

								output << lineBeg << *i << lineMid << val << lineEnd << "\n";
							}

							defaultCs.clear();
							continue;
						}
					}
					else if (line.find("</struct>") != string::npos)
					{
						changeActive = false;
					}
				}
				else if (line.find("\"name_id\"") != string::npos)
				{
					int begin = line.find(">") + 1;
					int end = line.find("<", begin);
					charcode = line.substr(begin, end - begin);

					if (maxSlots.find(charcode) != maxSlots.end())
					{
						changeActive = true;
					}
				}

				output << line << "\n";
			}

			input.close();
			output.close();

			fs::remove(fs::current_path() / "ui_chara_db.xml");
			fs::rename(fs::current_path() / "ui_chara_db_EDIT.xml", fs::current_path() / "ui_chara_db.xml");
		}
		else
		{
			if (!input.is_open())
			{
				log->LogText("> Error: " + fs::current_path().string() + "/ui_chara_db.xml could not be opened!");
			}

			if (!output.is_open())
			{
				log->LogText("> Error: " + fs::current_path().string() + "/ui_chara_db_EDIT.xml could not be opened!");
			}
		}
	}
}

void SmashData::patchXMLNames(map<string, map<int, Name>>& names)
{
	if (!names.empty())
	{
		ifstream uiVanilla("ui_chara_db.xml");
		ofstream uiEdit("ui_chara_db_EDIT.xml");
		ofstream msg("msg_name.xmsbt", ios::out | ios::binary);

		if (uiVanilla.is_open() && uiEdit.is_open() && msg.is_open())
		{
			// UTF-16 LE BOM
			unsigned char smarker[2];
			smarker[0] = 0xFF;
			smarker[1] = 0xFE;

			msg << smarker[0];
			msg << smarker[1];

			msg.close();

			wofstream msgUTF("msg_name.xmsbt", ios::binary | ios::app);
			msgUTF.imbue(std::locale(msgUTF.getloc(), new std::codecvt_utf16<wchar_t, 0x10ffff, std::little_endian>));

			if (!msgUTF.is_open())
			{
				log->LogText("> " + fs::current_path().string() + "/msg_name.xmsbt could not be opened!");
				return;
			}

			outputUTF(msgUTF, "<?xml version=\"1.0\" encoding=\"utf-16\"?>");
			outputUTF(msgUTF, "\n<xmsbt>");

			string charcode;
			string line;

			while (!uiVanilla.eof())
			{
				getline(uiVanilla, line);

				if (line.find("\"name_id\"") != string::npos)
				{
					int begin = line.find(">") + 1;
					int end = line.find("<", begin);
					charcode = line.substr(begin, end - begin);

					auto charIter = names.find(charcode);

					if (charIter != names.end())
					{
						uiEdit << line << "\n";

						string tempLine;

						auto i = charIter->second.begin();
						while (i != charIter->second.end())
						{
							if (tempLine.find("\"n07_index\"") == string::npos)
							{
								getline(uiVanilla, line);
								tempLine = line;

								// Output first n07 if not in change list
								if (tempLine.find("\"n07_index\"") != string::npos && i->first != 7)
								{
									uiEdit << line << "\n";
								}
							}

							string label;

							if (i->first > 7)
							{
								label = "\"n07_index\"";
							}
							else
							{
								label = "\"n0" + to_string(i->first) + "_index\"";
							}

							if (tempLine.find(label) != string::npos)
							{
								string slot;
								string nameSlot = to_string(i->first + 8);

								if (nameSlot.size() == 1)
								{
									nameSlot = "0" + nameSlot;
								}

								if (i->first > 9)
								{
									slot = to_string(i->first);
								}
								else
								{
									slot = "0" + to_string(i->first);
								}

								line = "\t<byte hash=\"n" + slot + "_index\">" + nameSlot + "</byte>";

								if (nameSlot == "08")
								{
									outputUTF(msgUTF, "\n\t<entry label=\"nam_chr3_" + nameSlot + "_" + charcode + "\">");
									outputUTF(msgUTF, "\n\t\t<text>");
									outputUTF(msgUTF, i->second.cssName, true);
									outputUTF(msgUTF, "</text>");
									outputUTF(msgUTF, "\n\t</entry>");
								}

								outputUTF(msgUTF, "\n\t<entry label=\"nam_chr1_" + nameSlot + "_" + charcode + "\">");
								outputUTF(msgUTF, "\n\t\t<text>");
								outputUTF(msgUTF, i->second.cspName, true);
								outputUTF(msgUTF, "</text>");
								outputUTF(msgUTF, "\n\t</entry>");

								outputUTF(msgUTF, "\n\t<entry label=\"nam_chr2_" + nameSlot + "_" + charcode + "\">");
								outputUTF(msgUTF, "\n\t\t<text>");
								outputUTF(msgUTF, i->second.vsName, true);
								outputUTF(msgUTF, "</text>");
								outputUTF(msgUTF, "\n\t</entry>");

								if (charcode != "eflame_first" && charcode != "elight_first")
								{
									outputUTF(msgUTF, "\n\t<entry label=\"nam_stage_name_" + nameSlot + "_" + charcode + "\">");
									outputUTF(msgUTF, "\n\t\t<text>");
									outputUTF(msgUTF, i->second.stageName, true);
									outputUTF(msgUTF, "</text>");
									outputUTF(msgUTF, "\n\t</entry>");
								}

								i++;
							}

							uiEdit << line << "\n";
						}
					}
				}
				else
				{
					uiEdit << line << "\n";
				}
			}

			outputUTF(msgUTF, "\n</xmsbt>");

			uiVanilla.close();
			uiEdit.close();
			msgUTF.close();

			fs::remove(fs::current_path() / "ui_chara_db.xml");
			fs::rename(fs::current_path() / "ui_chara_db_EDIT.xml", fs::current_path() / "ui_chara_db.xml");

			fs::create_directories(rootPath + "/ui/message/");
			fs::rename(fs::current_path() / "msg_name.xmsbt", rootPath + "/ui/message/msg_name.xmsbt");
		}
		else
		{
			if (!uiVanilla.is_open())
			{
				log->LogText("> Error: " + fs::current_path().string() + "/ui_chara_db.xml could not be opened!");
			}

			if (!uiEdit.is_open())
			{
				log->LogText("> Error: " + fs::current_path().string() + "/ui_chara_db_EDIT.xml could not be opened!");
			}

			if (!msg.is_open())
			{
				log->LogText("> Error: " + fs::current_path().string() + "/msg_name.xmsbt could not be opened!");
			}
		}
	}
}

void SmashData::patchXMLAnnouncer(map<string, map<int, string>>& announcers)
{
	if (!announcers.empty())
	{
		ifstream uiVanilla("ui_chara_db.xml");
		ofstream uiEdit("ui_chara_db_EDIT.xml");

		if (uiVanilla.is_open() && uiEdit.is_open())
		{
			string charcode;
			string line;

			while (!uiVanilla.eof())
			{
				getline(uiVanilla, line);

				if (line.find("\"name_id\"") != string::npos)
				{
					int begin = line.find(">") + 1;
					int end = line.find("<", begin);
					charcode = line.substr(begin, end - begin);

					auto charIter = announcers.find(charcode);

					if (charIter != announcers.end())
					{
						uiEdit << line << "\n";

						vector<string> vanillaLabels;
						string label = "";
						bool action = false;

						// Deal with characall_label
						while (line.find("</struct>") == string::npos)
						{
							getline(uiVanilla, line);
							uiEdit << line << endl;

							// Store vanillaLabels
							if (line.find("\"characall_label") != string::npos)
							{
								auto startPos = line.find('>') + 1;
								vanillaLabels.push_back(line.substr(startPos, line.rfind("<") - startPos));
							}

							if (line.find("\"characall_label_c07\"") != string::npos)
							{
								label = "characall_label_c";
								action = true;
							}
							else if (line.find("\"characall_label_article_c07\"") != string::npos)
							{
								label = "characall_label_article_c";
								action = true;
							}

							if (action)
							{
								for (auto i = charIter->second.begin(); i != charIter->second.end(); i++)
								{
									string slot = (i->first + 8 > 9 ? "" : "0") + to_string(i->first + 8);

									if (i->second == "Default")
									{
										string temp = (i->first > 9 ? "" : "0") + to_string(i->first);

										// Find base slot
										if (i->first > 7)
										{
											for (auto j = baseSlots[charcode].begin(); j != baseSlots[charcode].end(); j++)
											{
												if (j->second.find(temp) != j->second.end())
												{
													temp = vanillaLabels[stoi(j->first)];
													break;
												}
											}
										}
										else
										{
											temp = vanillaLabels[i->first];
										}

										// Replace with 00's announcer if 0#'s announcer is empty
										if (temp == "")
										{
											temp = vanillaLabels[0];
										}

										line = "      <hash40 hash=\"" + label + slot + "\">" + temp + "</hash40>";
									}
									else
									{
										line = "      <hash40 hash=\"" + label + slot + "\">vc_narration_characall_" + i->second + "</hash40>";
									}

									uiEdit << line << "\n";
								}

								vanillaLabels.clear();
								action = false;
							}
						}
					}
				}
				else
				{
					uiEdit << line << endl;
				}
			}

			uiVanilla.close();
			uiEdit.close();

			fs::remove(fs::current_path() / "ui_chara_db.xml");
			fs::rename(fs::current_path() / "ui_chara_db_EDIT.xml", fs::current_path() / "ui_chara_db.xml");
		}
		else
		{
			if (!uiVanilla.is_open())
			{
				log->LogText("> Error: " + fs::current_path().string() + "/ui_chara_db.xml could not be opened!");
			}

			if (!uiEdit.is_open())
			{
				log->LogText("> Error: " + fs::current_path().string() + "/ui_chara_db_EDIT.xml could not be opened!");
			}
		}
	}
}

void SmashData::patchXMLInkColors(map<int, InklingColor>& inklingColors)
{
	if (!inklingColors.empty())
	{
		ifstream effectVanilla("effect.xml");
		ofstream effectEdit("effect_EDIT.xml");

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

					bool lastStruct = false;
					int slot = -1;

					effectEdit << line << endl;

					auto i = inklingColors.begin();
					while (i != inklingColors.end())
					{
						if (!lastStruct)
						{
							getline(effectVanilla, line);
							slot = stoi(line.substr(line.find("\"") + 1, line.rfind("\"") - line.find("\"") - 1));
						}

						if (i->first == slot || lastStruct)
						{
							// <struct index = "[X]">
							effectEdit << "    <struct index=\"" << to_string(i->first) << "\">" << endl;

							if (action == 'E')
							{
								effectEdit << "      <float hash=\"r\">" << (i->second.effect.Red() / 255.0) << "</float>" << endl;
								effectEdit << "      <float hash=\"g\">" << (i->second.effect.Green() / 255.0) << "</float>" << endl;
								effectEdit << "      <float hash=\"b\">" << (i->second.effect.Blue() / 255.0) << "</float>" << endl;
							}
							else
							{
								effectEdit << "      <float hash=\"r\">" << (i->second.arrow.Red() / 255.0) << "</float>" << endl;
								effectEdit << "      <float hash=\"g\">" << (i->second.arrow.Green() / 255.0) << "</float>" << endl;
								effectEdit << "      <float hash=\"b\">" << (i->second.arrow.Blue() / 255.0) << "</float>" << endl;
							}

							if (!lastStruct)
							{
								// Read r, g, b, struct
								for (int i = 0; i < 4; i++)
								{
									getline(effectVanilla, line);
								}
							}

							effectEdit << "    </struct>" << endl;

							i++;
						}
						else
						{
							// <struct index = "[X]">
							effectEdit << line << endl;

							// Read and write r, g, b, struct
							for (int i = 0; i < 4; i++)
							{
								getline(effectVanilla, line);
								effectEdit << line << endl;
							}

							if (slot == 7)
							{
								lastStruct = true;
							}
						}
					}
				}
				else
				{
					effectEdit << line << endl;
				}
			}

			effectVanilla.close();
			effectEdit.close();

			fs::remove(fs::current_path() / "effect.xml");
			fs::rename(fs::current_path() / "effect_EDIT.xml", fs::current_path() / "effect.xml");
		}
		else
		{
			if (!effectVanilla.is_open())
			{
				log->LogText("> Error: " + fs::current_path().string() + "/effect.xml could not be opened!");
			}

			if (!effectEdit.is_open())
			{
				log->LogText("> Error: " + fs::current_path().string() + "/effect_EDIT.xml could not be opened!");
			}
		}
	}
}

void SmashData::createPRCXML(map<string, map<int, Name>>& names, map<string, map<int, string>>& announcers, map<string, int>& maxSlots)
{
	if (!maxSlots.empty() || !announcers.empty() || !names.empty())
	{
		ifstream uiVanilla("ui_chara_db.xml");
		ofstream uiEdit("ui_chara_db.prcxml");
		ofstream msg;

		if (!names.empty())
		{
			msg.open("msg_name.xmsbt", ios::out | ios::binary);

			if (msg.is_open())
			{
				// UTF-16 LE BOM
				unsigned char smarker[2];
				smarker[0] = 0xFF;
				smarker[1] = 0xFE;

				msg << smarker[0];
				msg << smarker[1];

				msg.close();
			}
			else
			{
				log->LogText("> Error: " + fs::current_path().string() + "/msg_name.xmsbt could not be opened!");
			}
		}

		if (uiVanilla.is_open() && uiEdit.is_open())
		{
			wofstream msgUTF;
			if (!names.empty())
			{
				msgUTF.open("msg_name.xmsbt", ios::binary | ios::app);
				msgUTF.imbue(std::locale(msgUTF.getloc(), new std::codecvt_utf16<wchar_t, 0x10ffff, std::little_endian>));

				if (!msgUTF.is_open())
				{
					log->LogText("> " + fs::current_path().string() + "/msg_name.xmsbt could not be opened!");
					return;
				}

				outputUTF(msgUTF, "<?xml version=\"1.0\" encoding=\"utf-16\"?>");
				outputUTF(msgUTF, "\n<xmsbt>");
			}

			uiEdit << "<?xml version=\"1.0\" encoding=\"UTF-16\"?>";
			uiEdit << "\n<struct>";
			uiEdit << "\n\t<list hash=\"db_root\">";

			string charcode;
			string line;
			string currIndex = "";
			char status = 0;

			while (!uiVanilla.eof())
			{
				getline(uiVanilla, line);

				if (line.find("<struct index=") != string::npos)
				{
					// TODO: Make efficent
					currIndex = line.substr(line.find('"') + 1, line.rfind('"') - line.find('"') - 1);
				}
				else if (line.find("\"name_id\"") != string::npos)
				{
					int begin = line.find(">") + 1;
					int end = line.find("<", begin);
					charcode = line.substr(begin, end - begin);

					// Deal with max-slots first
					if (maxSlots.find(charcode) != maxSlots.end())
					{
						status = 1;
						uiEdit << "\n\t\t<struct index=\"" << currIndex << "\">";
						uiEdit << "\n\t\t\t<byte hash=\"color_num\">" << to_string(maxSlots[charcode]) << "</byte>";
					}

					// Deal with names second
					if (!names.empty() && names.find(charcode) != names.end())
					{
						if (status != 1)
						{
							uiEdit << "\n\t\t<struct index=\"" << currIndex << "\">";
							status = 1;
						}
						auto charIter = names.find(charcode);

						auto i = charIter->second.begin();
						while (i != charIter->second.end())
						{
							string slot;
							string nameSlot = to_string(i->first + 8);

							if (nameSlot.size() == 1)
							{
								nameSlot = "0" + nameSlot;
							}

							if (i->first > 9)
							{
								slot = to_string(i->first);
							}
							else
							{
								slot = "0" + to_string(i->first);
							}

							uiEdit << "\n\t\t\t<byte hash=\"n" << slot << "_index\">" << nameSlot << "</byte>";

							if (nameSlot == "08")
							{
								outputUTF(msgUTF, "\n\t<entry label=\"nam_chr3_" + nameSlot + "_" + charcode + "\">");
								outputUTF(msgUTF, "\n\t\t<text>");
								outputUTF(msgUTF, i->second.cssName, true);
								outputUTF(msgUTF, "</text>");
								outputUTF(msgUTF, "\n\t</entry>");
							}

							outputUTF(msgUTF, "\n\t<entry label=\"nam_chr1_" + nameSlot + "_" + charcode + "\">");
							outputUTF(msgUTF, "\n\t\t<text>");
							outputUTF(msgUTF, i->second.cspName, true);
							outputUTF(msgUTF, "</text>");
							outputUTF(msgUTF, "\n\t</entry>");

							outputUTF(msgUTF, "\n\t<entry label=\"nam_chr2_" + nameSlot + "_" + charcode + "\">");
							outputUTF(msgUTF, "\n\t\t<text>");
							outputUTF(msgUTF, i->second.vsName, true);
							outputUTF(msgUTF, "</text>");
							outputUTF(msgUTF, "\n\t</entry>");

							if (charcode != "eflame_first" && charcode != "elight_first")
							{
								outputUTF(msgUTF, "\n\t<entry label=\"nam_stage_name_" + nameSlot + "_" + charcode + "\">");
								outputUTF(msgUTF, "\n\t\t<text>");
								outputUTF(msgUTF, i->second.stageName, true);
								outputUTF(msgUTF, "</text>");
								outputUTF(msgUTF, "\n\t</entry>");
							}

							i++;
						}
					}

					// Deal with announcers third
					if (!announcers.empty() && announcers.find(charcode) != announcers.end())
					{
						if (status != 1)
						{
							uiEdit << "\n\t\t<struct index=\"" << currIndex << "\">";
							status = 1;
						}
						auto charIter = announcers.find(charcode);

						vector<string> vanillaLabels;
						string label = "";
						bool action = false;

						// Deal with characall_label
						while (line.find("shop_item_tag") == string::npos)
						{
							getline(uiVanilla, line);

							// Store vanillaLabels
							if (line.find("\"characall_label") != string::npos)
							{
								auto startPos = line.find('>') + 1;
								vanillaLabels.push_back(line.substr(startPos, line.rfind("<") - startPos));
							}

							if (line.find("\"characall_label_c07\"") != string::npos)
							{
								label = "characall_label_c";
								action = true;
							}
							else if (line.find("\"characall_label_article_c07\"") != string::npos)
							{
								label = "characall_label_article_c";
								action = true;
							}

							if (action)
							{
								for (auto i = charIter->second.begin(); i != charIter->second.end(); i++)
								{
									string slot = (i->first + 8 > 9 ? "" : "0") + to_string(i->first + 8);

									if (i->second == "Default")
									{
										string temp = (i->first > 9 ? "" : "0") + to_string(i->first);

										// Find base slot
										if (i->first > 7)
										{
											for (auto j = baseSlots[charcode].begin(); j != baseSlots[charcode].end(); j++)
											{
												if (j->second.find(temp) != j->second.end())
												{
													temp = vanillaLabels[stoi(j->first)];
													break;
												}
											}
										}
										else
										{
											temp = vanillaLabels[i->first];
										}

										// Replace with 00's announcer if 0#'s announcer is empty
										if (temp == "")
										{
											temp = vanillaLabels[0];
										}

										line = "<hash40 hash=\"" + label + slot + "\">" + temp + "</hash40>";
									}
									else
									{
										line = "<hash40 hash=\"" + label + slot + "\">vc_narration_characall_" + i->second + "</hash40>";
									}

									uiEdit << "\n\t\t\t" << line;
								}

								vanillaLabels.clear();
								action = false;
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

					// Skip everything else as demon is the last charcode.
					if (currIndex == "120")
					{
						break;
					}
				}
			}

			if (!names.empty())
			{
				outputUTF(msgUTF, "\n</xmsbt>");
				msgUTF.close();

				fs::create_directories(rootPath + "/ui/message/");
				fs::rename(fs::current_path() / "msg_name.xmsbt", rootPath + "/ui/message/msg_name.xmsbt");
			}


			uiEdit << "\n\t</list>";
			uiEdit << "\n</struct>";

			uiVanilla.close();
			uiEdit.close();

			fs::remove(fs::current_path() / "ui_chara_db.xml");
		}
		else
		{
			if (!uiVanilla.is_open())
			{
				log->LogText("> Error: " + fs::current_path().string() + "/ui_chara_db.xml could not be opened!");
			}

			if (!uiEdit.is_open())
			{
				log->LogText("> Error: " + fs::current_path().string() + "/ui_chara_db_EDIT.xml could not be opened!");
			}
		}
	}
}

void SmashData::outputUTF(wofstream& file, string str, bool parse)
{
	if (parse)
	{
		string result = "";

		for (auto i = str.begin(); i != str.end(); i++)
		{
			switch (*i)
			{
				case '"':
				{
					result += "&quot;";
					break;
				}
				case '\'':
				{
					result += "&apos;";
					break;
				}
				case '&':
				{
					result += "&amp;";
					break;
				}
				case '<':
				{
					result += "&lt;";
					break;
				}
				case '>':
				{
					result += "&gt;";
					break;
				}
				case '|':
				{
					result += '\n';
					break;
				}
				default:
				{
					result += *i;
				}
			}
		}

		file << std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>>{}.from_bytes(result);
	}
	else
	{
		file << std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>>{}.from_bytes(str);
	}
}

map<string, map<string, string>> SmashData::readBaseSlots()
{
	map<string, map<string, string>> baseSlots;

	if (fs::exists(rootPath + "/config.json"))
	{
		ifstream inFile(rootPath + "/config.json");

		if (inFile.is_open())
		{
			string line;

			while (line.find("\"new-dir-infos-base\"") == string::npos && !inFile.eof())
			{
				getline(inFile, line);
			}

			// Found base-slot section
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
						string charcode = line.substr(beg, end - beg);

						string newSlot = line.substr(end + 2, camPos - end - 2);

						end = line.find("/camera", camPos + 7) - 1;
						beg = line.find(charcode + "/c", camPos + 7) + 2 + charcode.size();
						string oldSlot = line.substr(beg, end - beg + 1);

						log->LogText(charcode + " " + newSlot + " " + oldSlot);

						baseSlots[charcode][newSlot] = oldSlot;
					}
				}
			}
		}
		else
		{
			log->LogText("> ERROR: " + rootPath + "/config.json" + "could not be opened!");
		}
	}

	return baseSlots;
}

map<string, map<string, Name>> SmashData::readNames()
{
	map<string, map<string, Name>> names;
	int count = 0;

	if (fs::exists(rootPath + "/ui/message/msg_name.xmsbt"))
	{
		wifstream inFile(rootPath + "/ui/message/msg_name.xmsbt", ios::binary);
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
					beg = lines[i].find("nam_chr") + 10;
					end = lines[i].find("_", beg);
					type = lines[i][beg - 3];

					action = true;
				}
				// Stage_Name
				else if (lines[i].find("nam_stage") != string::npos)
				{
					beg = lines[i].find("nam_stage") + 16;
					end = lines[i].find("_", beg);
					type = 's';

					action = true;
				}

				if (action)
				{
					string slot = to_string(stoi(lines[i].substr(beg, end - beg)) - 8);
					string charcode = lines[i].substr(end + 1, lines[i].find("\"", end + 1) - end - 1);

					if (slot.size() == 1)
					{
						slot = "0" + slot;
					}
					string name = lines[i + 2].substr(0, lines[i + 2].find("<"));

					if (type == '1')
					{
						names[charcode][slot].cspName = name;
					}
					else if (type == '2')
					{
						names[charcode][slot].vsName = name;
					}
					else if (type == '3')
					{
						names[charcode][slot].cssName = name;
					}
					else if (type == 's')
					{
						names[charcode][slot].stageName = name;
					}
					else
					{
						// TODO: ERROR
					}

					log->LogText(name);

					action = false;
					i += 2;
				}
			}

			inFile.close();
		}
		else
		{
			log->LogText("> ERROR: " + rootPath + "/ui/message/msg_name.xmsbt" + "could not be opened!");
		}
	}

	return names;
}

void SmashData::clear()
{
	for (auto i = mod.begin(); i != mod.end(); i++)
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

	mod.clear();
}
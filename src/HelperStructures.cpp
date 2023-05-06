#include "HelperStructures.h"

/* --- {Slot} ---*/
Slot::Slot()
{
	slot = -1;
}

Slot::Slot(int slot)
{
	if (slot >= 0)
	{
		this->slot = slot;
	}
	else
	{
		this->slot = -1;
	}
}

Slot::Slot(string slot)
{
	try
	{
		if (slot == "All")
		{
			this->slot = 999;
			return;
		}
		else if (slot.size() > 1 && slot[0] == 'c')
		{
			slot = slot.substr(1);
		}

		this->slot = stoi(slot);

		if (this->slot < 0)
		{
			this->slot = -1;
		}
	}
	catch (...)
	{
		this->slot = -1;
	}
}

int Slot::getInt() const
{
	return slot;
}

string Slot::getString() const
{
	if (slot >= 0)
	{
		if (slot == 999)
		{
			return "All";
		}

		return (slot < 10) ? "0" + to_string(slot) : to_string(slot);
	}
	else
	{
		return "";
	}
}

void Slot::set(Slot slot)
{
	this->slot = slot.getInt();
}

void Slot::set(int slot)
{
	this->set(Slot(slot));
}

void Slot::set(string slot)
{
	this->set(Slot(slot));
}

// Priority: -1, 999, # in ascending order
bool Slot::operator<(const Slot& rhs) const
{
	if (this->slot != rhs.slot)
	{
		if (this->slot == -1)
		{
			return true;
		}
		else if (rhs.slot == -1)
		{
			return false;
		}
		else if (this->slot == 999)
		{
			return true;
		}
		else if (rhs.slot == 999)
		{
			return false;
		}
	}
	return this->slot < rhs.slot;	
}

/* --- {Path} ---*/
Path::Path(string path)
{
	replace(path.begin(), path.end(), '\\', '/');

	this->path = path;
	this->type = "";
	this->slot = Slot();

	if (path.find("effect") == 0)
	{
		this->type = "effect";

		if (path.find("/fighter/") != string::npos)
		{
			auto effPos = path.rfind(".eff");
			if (effPos != string::npos)
			{
				auto underscorePos = path.rfind("_c");
				if (underscorePos != string::npos && underscorePos < effPos)
				{
					try
					{
						size_t* temp = new size_t;
						this->slot = Slot(stoi(path.substr(underscorePos + 2, effPos - underscorePos - 2), temp));
						if (*temp != effPos - underscorePos - 2)
						{
							this->slot = Slot(999);
						}
						delete temp;
					}
					catch (...)
					{
						// Slot was invalid
						this->slot = Slot(999);
					}
				}
				else
				{
					this->slot = Slot(999);
				}
			}
			else
			{
				auto modelPos = path.find("/model/");
				auto begPos = (modelPos != string::npos) ? path.find('/', modelPos + 7) : path.rfind('/');

				if (modelPos == string::npos || begPos != string::npos)
				{
					auto underscorePos = (modelPos != string::npos) ? path.rfind("_c", begPos) : path.rfind("_c");
					if (underscorePos != string::npos && (modelPos != string::npos) ? (underscorePos < begPos) : (underscorePos > begPos))
					{
						try
						{
							size_t* temp = new size_t;
							if (modelPos != string::npos)
							{
								this->slot = Slot(stoi(path.substr(underscorePos + 2, begPos - underscorePos - 2), temp));
							}
							else
							{
								this->slot = Slot(stoi(path.substr(underscorePos + 2), temp));
								begPos = path.size();
							}
							if (*temp != begPos - underscorePos - 2)
							{
								this->slot = Slot(999);
							}
							delete temp;
						}
						catch (...)
						{
							this->slot = Slot(999);
						}
					}
					else
					{
						this->slot = Slot(999);
					}
				}
			}
		}
	}
	else if (path.find("fighter") == 0 || path.find("camera") == 0)
	{
		this->type = path.find("fighter") == 0 ? "fighter" : "camera";

		int index = 0;
		while (index < path.size() - 1)
		{
			auto cPos = path.find("/c", index);
			if (cPos != string::npos)
			{
				auto endPos = path.find('/', cPos + 1);
				if (endPos != string::npos && cPos < endPos)
				{
					try
					{
						size_t* temp = new size_t;
						this->slot = stoi(path.substr(cPos + 2, endPos - cPos - 2), temp);

						if (*temp != endPos - cPos - 2)
						{
							this->slot = Slot();
						}
						else
						{
							delete temp;
							break;
						}

						delete temp;
					}
					catch (...)
					{
						index = endPos;
						continue;
					}
				}
				else
				{
					break;
				}
			}
			else
			{
				break;
			}

			index++;
		}

		if (slot.getInt() == -1)
		{
			auto endPos = path.rfind('/c');
			if (endPos != string::npos)
			{
				try
				{
					size_t* temp = new size_t;
					this->slot = stoi(path.substr(endPos + 1, path.size() - endPos - 1), temp);

					if (*temp != path.size() - endPos - 1)
					{
						this->slot = Slot();
					}
					delete temp;
				}
				catch (...)
				{
					// Invalid slot
				}
			}
		}
	}
	else if (path.find("sound") == 0)
	{
		this->type = "sound";

		if (path.find("/fighter/") != string::npos || path.find("/fighter_voice/") != string::npos)
		{
			auto dotPos = path.rfind(".");

			if (path.find("/bank/") != string::npos && dotPos != string::npos)
			{
				auto underscorePos = path.rfind("_c");

				if (underscorePos != string::npos && underscorePos - dotPos)
				{
					try
					{
						size_t* temp = new size_t;
						this->slot = Slot(stoi(path.substr(underscorePos + 2, dotPos - underscorePos - 2), temp));
						if (*temp != dotPos - underscorePos - 2)
						{
							this->slot = Slot();
						}
						delete temp;
					}
					catch (...)
					{
						// Slot was invalid?
					}
				}
				else
				{
					this->slot = Slot(999);
				}
			}
		}
		else
		{
			// File that is not currently supported
		}
	}
	else if (path.find("ui") == 0)
	{
		this->type = "ui";

		if (path.find("/replace/") != string::npos || path.find("/replace_patch/") != string::npos)
		{
			auto dotPos = path.rfind(".bntx");

			if (path.find("/chara/") != string::npos && dotPos != string::npos)
			{
				auto underscorePos = path.rfind("_");

				if (underscorePos != string::npos && underscorePos - dotPos)
				{
					try
					{
						size_t* temp = new size_t;
						this->slot = Slot(stoi(path.substr(underscorePos + 1, dotPos - underscorePos - 1), temp));
						if (*temp != dotPos - underscorePos - 1)
						{
							this->slot = Slot();
						}
						delete temp;
					}
					catch (...)
					{
						// Slot was invalid?
					}
				}
			}
		}
		else
		{
			// File that is not currently supported
		}
	}
}

string Path::getPath() const
{
	return path;
}

string Path::getType() const
{
	return type;
}

Slot Path::getSlot() const
{
	return slot;
}

void Path::setSlot(Slot slot)
{
	if (this->slot.getInt() != -1 && this->slot.getInt() != slot.getInt())
	{
		if (this->type == "effect")
		{
			if (this->slot.getInt() == 999)
			{
				if (path.find(".eff") != string::npos)
				{
					path.replace(path.rfind('.'), 1, "_c" + slot.getString() + ".");
				}
				else
				{
					auto modelPos = path.find("/model/");
					if (modelPos != string::npos)
					{
						path.replace(path.find('/', modelPos + 7), 1, "_c" + slot.getString() + "/");
					}
					else
					{
						if (path.find(".") != string::npos)
						{
							path.replace(path.rfind('/'), 1, "_c" + slot.getString() + "/");
						}
						else
						{
							path += "_c" + slot.getString();
						}
					}
				}
			}
			else
			{
				string toReplace = "_c" + this->slot.getString();
				if (slot.getInt() == 999)
				{
					path.replace(path.find(toReplace), toReplace.size(), "");
				}
				else
				{
					path.replace(path.rfind(toReplace), toReplace.size(), "_c" + slot.getString());
				}
			}
		}
		else if (this->type == "fighter" || this->type == "camera")
		{
			string toReplace = "/c" + this->slot.getString();
			path.replace(path.find(toReplace), toReplace.size(), "/c" + slot.getString());
		}
		else if (this->type == "sound")
		{
			if (this->slot.getInt() == 999)
			{
				path.replace(path.rfind('.'), 1, "_c" + slot.getString() + ".");
			}
			else
			{
				string toReplace = "_c" + this->slot.getString() + ".";
				if (slot.getInt() == 999)
				{
					path.replace(path.find(toReplace), toReplace.size(), ".");
				}
				else
				{
					path.replace(path.find(toReplace), toReplace.size(), "_c" + slot.getString() + ".");
				}
			}
		}
		else if (this->type == "ui")
		{
			string toReplace = "_" + this->slot.getString() + ".";
			path.replace(path.find(toReplace), toReplace.size(), "_" + slot.getString() + ".");
		}

		this->slot.set(slot);
	}
}

void Path::setSlot(int slot)
{
	this->setSlot(Slot(slot));
}

void Path::setSlot(string slot)
{
	this->setSlot(Slot(slot));
}

// Priority: Alphabetic
bool Path::operator<(const Path& rhs) const
{
	return this->path < rhs.path;
}

/* --- {DBData} ---*/
DBData::DBData()
{
	nIndex = 0;
	cIndex = 0;
	cGroup = 0;

	label = "";
	article = "";
}

DBData::DBData(string code)
{
	nIndex = 0;
	cIndex = 0;
	cGroup = 0;

	label = "vc_narration_characall_" + code;
	article = "";
}
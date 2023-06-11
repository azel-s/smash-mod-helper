#include "PrcSelection.h"
#include <wx/gbsizer.h>

PrcSelection::PrcSelection(wxWindow* parent, wxWindowID id,
	const wxString& title,
	ModHandler* mHandler,
	Settings settings,
	map<string, map<int, CssData>>* css,
	const wxPoint& pos,
	const wxSize& size,
	long style,
	const wxString& name) :
	wxDialog(parent, id, title, pos, size, style, name)
{
	this->mHandler = mHandler;

	auto panel = new wxScrolled<wxPanel>(this, wxID_ANY, wxDefaultPosition, wxDefaultSize);
	panel->SetScrollRate(0, 10);

	wxBoxSizer* sizerM = new wxBoxSizer(wxVERTICAL);
	wxGridBagSizer* gridSizer = new wxGridBagSizer(5, 20);
	wxTextCtrl* textCtrl = new wxTextCtrl();
	wxStaticText* text;

	wxFont* boldFont = new wxFont();
	boldFont->SetWeight(wxFONTWEIGHT_BOLD);

	gridSizer->SetFlexibleDirection(wxBOTH);

	allSlots = mHandler->getAllSlots(false);

	if (allSlots.find("kirby") != allSlots.end() && mHandler->isKirbyCopyOnly())
	{
		allSlots.extract("kirby");
	}

	for (auto i = allSlots.begin(); i != allSlots.end(); i++)
	{
		if (mHandler->getName(i->first).empty())
		{
			allSlots.extract(i->first);
		}
	}

	if (css || settings.readBase)
	{
		map<string, map<int, CssData>> tmp;

		if (!css)
		{
			tmp = mHandler->read_prcxml_css();
			css = &tmp;
		}

		for (auto i = css->begin(); i != css->end(); i++)
		{
			for (auto j = i->second.begin(); j != i->second.end(); j++)
			{
				if (allSlots.find(j->second.code) != allSlots.end())
				{
					allSlots.extract(j->second.code);
				}

				for (int k = 0; k < j->second.color_num; k++)
				{
					allSlots[j->second.code].insert(Slot(k));
				}
			}
		}
	}

	auto prevNames = mHandler->read_message_names();
	bool slots = mHandler->hasAddSlot();

	int rpm; // Aegis max slot.
	{
		bool hasRex = mHandler->hasChar("element");
		bool hasPyra = mHandler->hasChar("eflame");
		bool hasMythra = mHandler->hasChar("elight");

		int r = hasRex ? allSlots.find("element")->second.rbegin()->getInt() + 1 : 0;
		int p = hasRex ? allSlots.find("eflame")->second.rbegin()->getInt() + 1 : 0;
		int m = hasRex ? allSlots.find("elight")->second.rbegin()->getInt() + 1 : 0;

		rpm = max({ r, p, m });
	}

	wxString fieldName;

	bool hasChar = false;
	bool hasSlot = false;

	// Create Header
	text = new wxStaticText(panel, wxID_ANY, "Characters");
	text->SetFont(*boldFont);
	gridSizer->Add(text, wxGBPosition(0, 0));

	// Check if slot 00 exists anywhere
	bool hasSlot00 = false;
	for (auto i = allSlots.begin(); i != allSlots.end(); i++)
	{
		if (i->second.find(Slot(0)) != i->second.end())
		{
			hasSlot00 = true;
			break;
		}
	}

	if (slots)
	{
		text = new wxStaticText(panel, wxID_ANY, "Max Slots");
		text->SetFont(*boldFont);
		gridSizer->Add(text, wxGBPosition(0, 1), wxGBSpan(), wxALIGN_CENTER_HORIZONTAL);
	}

	int currCol = 2;

	text = new wxStaticText(panel, wxID_ANY, "Slot");
	text->SetFont(*boldFont);
	gridSizer->Add(text, wxGBPosition(0, currCol), wxGBSpan(), wxALIGN_CENTER_HORIZONTAL);
	currCol++;

	if (hasSlot00)
	{
		text = new wxStaticText(panel, wxID_ANY, "CSS Name");
		text->SetFont(*boldFont);
		gridSizer->Add(text, wxGBPosition(0, currCol), wxGBSpan(), wxALIGN_CENTER_HORIZONTAL);
		currCol++;
	}

	text = new wxStaticText(panel, wxID_ANY, "CSP Name");
	text->SetFont(*boldFont);
	gridSizer->Add(text, wxGBPosition(0, currCol), wxGBSpan(), wxALIGN_CENTER_HORIZONTAL);
	currCol++;

	text = new wxStaticText(panel, wxID_ANY, "VS/Results Name");
	text->SetFont(*boldFont);
	gridSizer->Add(text, wxGBPosition(0, currCol), wxGBSpan(), wxALIGN_CENTER_HORIZONTAL);
	currCol++;

	text = new wxStaticText(panel, wxID_ANY, "Boxing Ring Name");
	text->SetFont(*boldFont);
	gridSizer->Add(text, wxGBPosition(0, currCol), wxGBSpan(), wxALIGN_CENTER_HORIZONTAL);
	currCol++;

	text = new wxStaticText(panel, wxID_ANY, "Custom Announcer");
	text->SetFont(*boldFont);
	gridSizer->Add(text, wxGBPosition(0, currCol), wxGBSpan(), wxALIGN_CENTER_HORIZONTAL);
	currCol++;

	gridSizer->AddGrowableCol(3);
	gridSizer->AddGrowableCol(4);
	gridSizer->AddGrowableCol(5);
	gridSizer->AddGrowableCol(6);

	if (hasSlot00)
	{
		gridSizer->AddGrowableCol(7);
	}

	int currRow = 1;
	currCol = 0;

	// Create fields
	for (auto i = allSlots.begin(); i != allSlots.end(); i++)
	{
		string nameStr = mHandler->getName(i->first);
		text = new wxStaticText(panel, wxID_ANY, nameStr.empty() ? i->first : nameStr);

		if (i->first != "element")
		{
			gridSizer->Add(text, wxGBPosition(currRow, currCol), wxGBSpan(i->second.size(), 1), wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL);
		}
		else
		{
			gridSizer->Add(text, wxGBPosition(currRow, currCol), wxGBSpan(1, 1), wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL);
		}

		currCol++;

		if (slots)
		{
			wxSpinCtrl* userSlot;
			if (i->first == "element" || i->first == "eflame" || i->first == "elight")
			{
				userSlot = new wxSpinCtrl(panel, wxID_ANY, "", wxDefaultPosition, FromDIP(wxSize(50, -1)), wxSP_WRAP, 8, 255, rpm);
			}
			else
			{
				userSlot = new wxSpinCtrl(panel, wxID_ANY, "", wxDefaultPosition, FromDIP(wxSize(50, -1)), wxSP_WRAP, 8, 255, i->second.rbegin()->getInt() + 1);
			}

			maxSlots.push_back(userSlot);

			if (i->first != "element")
			{
				gridSizer->Add(userSlot, wxGBPosition(currRow, currCol), wxGBSpan(i->second.size(), 1), wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL);
			}
			else
			{
				gridSizer->Add(userSlot, wxGBPosition(currRow, currCol), wxGBSpan(1, 1), wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL);
			}
		}
		currCol++;

		if (i->first != "element")
		{
			string charcode = i->first;

			if (i->first == "eflame")
			{
				charcode = "eflame_only";
			}
			else if (i->first == "elight")
			{
				charcode = "elight_only";
			}

			if (settings.readNames && prevNames.find(charcode) != prevNames.end())
			{
				hasChar = true;
			}

			for (auto j = i->second.begin(); j != i->second.end(); j++)
			{
				auto message = mHandler->getMessage(charcode, *j);

				if (hasChar && prevNames[charcode].find(*j) != prevNames[charcode].end())
				{
					hasSlot = true;
				}

				text = new wxStaticText(panel, wxID_ANY, "c" + j->getString());
				gridSizer->Add(text, wxGBPosition(currRow, currCol), wxGBSpan(), wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL);
				currCol++;

				if (hasSlot00 && j->getInt() == 0)
				{
					if (hasSlot)
					{
						fieldName = prevNames[charcode][*j].cssName;
					}
					else
					{
						fieldName = message.cssName;
					}

					textCtrl = new wxTextCtrl(panel, wxID_ANY, fieldName, wxDefaultPosition, wxDefaultSize, wxTE_CENTRE);
					slotNames[charcode][*j].cssName = textCtrl;
					gridSizer->Add(textCtrl, wxGBPosition(currRow, currCol), wxGBSpan(), wxEXPAND | wxALIGN_CENTER_VERTICAL);
					currCol++;
				}
				else if (hasSlot00)
				{
					currCol++;
				}

				if (hasSlot)
				{
					fieldName = prevNames[charcode][*j].cspName;
				}
				else
				{
					fieldName = message.cspName;
				}

				textCtrl = new wxTextCtrl(panel, wxID_ANY, fieldName, wxDefaultPosition, wxDefaultSize, wxTE_CENTRE);
				slotNames[charcode][*j].cspName = textCtrl;
				gridSizer->Add(textCtrl, wxGBPosition(currRow, currCol), wxGBSpan(), wxEXPAND | wxALIGN_CENTER_VERTICAL);
				currCol++;

				if (hasSlot)
				{
					fieldName = prevNames[charcode][*j].vsName;
				}
				else
				{
					fieldName = message.vsName;
				}

				textCtrl = new wxTextCtrl(panel, wxID_ANY, fieldName, wxDefaultPosition, wxDefaultSize, wxTE_CENTRE);
				slotNames[charcode][*j].vsName = textCtrl;
				gridSizer->Add(textCtrl, wxGBPosition(currRow, currCol), wxGBSpan(), wxEXPAND | wxALIGN_CENTER_VERTICAL);
				currCol++;

				slotNames[charcode][*j].cspName->Bind(wxEVT_TEXT, &PrcSelection::onType, this, wxID_ANY, wxID_ANY, new prcArgument(textCtrl));

				if (charcode != "pzenigame" && charcode != "plizardon" && charcode != "pfushigisou")
				{
					if (hasSlot)
					{
						fieldName = prevNames[charcode][*j].stageName;
					}
					else
					{
						fieldName = message.stageName;
					}

					textCtrl = new wxTextCtrl(panel, wxID_ANY, fieldName, wxDefaultPosition, wxDefaultSize, wxTE_CENTRE);
					slotNames[charcode][*j].stageName = textCtrl;
					gridSizer->Add(textCtrl, wxGBPosition(currRow, currCol), wxGBSpan(), wxEXPAND | wxALIGN_CENTER_VERTICAL);
				}
				currCol++;

				textCtrl = new wxTextCtrl(panel, wxID_ANY, "Default", wxDefaultPosition, wxDefaultSize, wxTE_CENTRE);
				slotNames[charcode][*j].announcer = textCtrl;
				gridSizer->Add(textCtrl, wxGBPosition(currRow, currCol), wxGBSpan(), wxEXPAND | wxALIGN_CENTER_VERTICAL);
				currCol++;

				currRow++;
				currCol = 2;

				hasSlot = false;
			}

			// Pyra and Mythra both have same slots
			if (i->first == "elight" && allSlots.find("eflame") != allSlots.end())
			{
				currRow++;
				currCol = 0;

				// Get slot union
				set<Slot> slotUnion;
				auto eflameIter = allSlots.find("eflame");
				for (auto j = i->second.begin(); j != i->second.end(); j++)
				{
					if (eflameIter->second.find(*j) != eflameIter->second.end())
					{
						slotUnion.insert(*j);
					}
				}

				// Display Special names
				if (!slotUnion.empty())
				{
					bool hasFlame = false;
					bool hasLight = false;
					if (settings.readNames && prevNames.find("eflame_first") != prevNames.end())
					{
						hasFlame = true;
					}
					if (hasChar && prevNames.find("elight_first") != prevNames.end())
					{
						hasLight = true;
					}

					text = new wxStaticText(panel, wxID_ANY, "Pyra/Mythra");
					gridSizer->Add(text, wxGBPosition(currRow, currCol), wxGBSpan(slotUnion.size() * 2, 1), wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL);
					currCol++;

					text = new wxStaticText(panel, wxID_ANY, "Unique");
					gridSizer->Add(text, wxGBPosition(currRow, currCol), wxGBSpan(slotUnion.size() * 2, 1), wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL);
					currCol++;

					for (auto j = slotUnion.begin(); j != slotUnion.end(); j++)
					{
						auto messageF = mHandler->getMessage("eflame_first", *j);
						auto messageL = mHandler->getMessage("elight_first", *j);

						bool hasFlameSlot = false;

						if (hasFlame && prevNames["eflame_first"].find(*j) != prevNames["eflame_first"].end())
						{
							hasFlameSlot = true;
						}

						text = new wxStaticText(panel, wxID_ANY, "c" + j->getString() + " P/M");
						gridSizer->Add(text, wxGBPosition(currRow, currCol), wxGBSpan(), wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL);
						currCol++;

						if (hasSlot00 && j->getInt() == 0)
						{
							if (hasFlameSlot)
							{
								fieldName = prevNames["eflame_first"][*j].cssName;
							}
							else
							{
								fieldName = messageF.cssName;
							}

							textCtrl = new wxTextCtrl(panel, wxID_ANY, fieldName, wxDefaultPosition, wxDefaultSize, wxTE_CENTRE);
							slotNames["eflame_first"][*j].cssName = textCtrl;
							gridSizer->Add(textCtrl, wxGBPosition(currRow, currCol), wxGBSpan(), wxEXPAND | wxALIGN_CENTER_VERTICAL);
							currCol++;
						}
						else if (hasSlot00)
						{
							currCol++;
						}

						if (hasFlameSlot)
						{
							fieldName = prevNames["eflame_first"][*j].cspName;
						}
						else
						{
							fieldName = messageF.cspName;
						}

						textCtrl = new wxTextCtrl(panel, wxID_ANY, fieldName, wxDefaultPosition, wxDefaultSize, wxTE_CENTRE);
						slotNames["eflame_first"][*j].cspName = textCtrl;
						gridSizer->Add(textCtrl, wxGBPosition(currRow, currCol), wxGBSpan(), wxEXPAND | wxALIGN_CENTER_VERTICAL);
						currCol++;

						if (hasFlameSlot)
						{
							fieldName = prevNames["eflame_first"][*j].vsName;
						}
						else
						{
							fieldName = messageF.vsName;
						}

						textCtrl = new wxTextCtrl(panel, wxID_ANY, fieldName, wxDefaultPosition, wxDefaultSize, wxTE_CENTRE);
						slotNames["eflame_first"][*j].vsName = textCtrl;
						gridSizer->Add(textCtrl, wxGBPosition(currRow, currCol), wxGBSpan(), wxEXPAND | wxALIGN_CENTER_VERTICAL);
						currCol += 2;

						slotNames["eflame_first"][*j].cspName->Bind(wxEVT_TEXT, &PrcSelection::onType, this, wxID_ANY, wxID_ANY, new prcArgument(textCtrl));

						textCtrl = new wxTextCtrl(panel, wxID_ANY, "Default", wxDefaultPosition, wxDefaultSize, wxTE_CENTRE);
						slotNames["eflame_first"][*j].announcer = textCtrl;
						gridSizer->Add(textCtrl, wxGBPosition(currRow, currCol), wxGBSpan(), wxEXPAND | wxALIGN_CENTER_VERTICAL);

						currRow++;
						currCol = 2;

						text = new wxStaticText(panel, wxID_ANY, "c" + j->getString() + " M/P");
						gridSizer->Add(text, wxGBPosition(currRow, currCol), wxGBSpan(), wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL);
						currCol++;

						bool hasLightSlot = false;

						if (hasLight && prevNames["elight_first"].find(*j) != prevNames["elight_first"].end())
						{
							hasLightSlot = true;
						}

						if (hasSlot00 && j->getInt() == 0)
						{
							if (hasLightSlot)
							{
								fieldName = prevNames["elight_first"][*j].cssName;
							}
							else
							{
								fieldName = messageL.cssName;
							}

							textCtrl = new wxTextCtrl(panel, wxID_ANY, fieldName, wxDefaultPosition, wxDefaultSize, wxTE_CENTRE);
							slotNames["elight_first"][*j].cssName = textCtrl;
							gridSizer->Add(textCtrl, wxGBPosition(currRow, currCol), wxGBSpan(), wxEXPAND | wxALIGN_CENTER_VERTICAL);
							currCol++;
						}
						else if (hasSlot00)
						{
							currCol++;
						}

						if (hasLightSlot)
						{
							fieldName = prevNames["elight_first"][*j].cspName;
						}
						else
						{
							fieldName = messageL.cspName;
						}

						textCtrl = new wxTextCtrl(panel, wxID_ANY, fieldName, wxDefaultPosition, wxDefaultSize, wxTE_CENTRE);
						slotNames["elight_first"][*j].cspName = textCtrl;
						gridSizer->Add(textCtrl, wxGBPosition(currRow, currCol), wxGBSpan(), wxEXPAND | wxALIGN_CENTER_VERTICAL);
						currCol++;

						if (hasLightSlot)
						{
							fieldName = prevNames["elight_first"][*j].vsName;
						}
						else
						{
							fieldName = messageL.vsName;
						}

						textCtrl = new wxTextCtrl(panel, wxID_ANY, fieldName, wxDefaultPosition, wxDefaultSize, wxTE_CENTRE);
						slotNames["elight_first"][*j].vsName = textCtrl;
						gridSizer->Add(textCtrl, wxGBPosition(currRow, currCol), wxGBSpan(), wxEXPAND | wxALIGN_CENTER_VERTICAL);
						currCol += 2;

						slotNames["elight_first"][*j].cspName->Bind(wxEVT_TEXT, &PrcSelection::onType, this, wxID_ANY, wxID_ANY, new prcArgument(textCtrl));

						textCtrl = new wxTextCtrl(panel, wxID_ANY, "Default", wxDefaultPosition, wxDefaultSize, wxTE_CENTRE);
						slotNames["elight_first"][*j].announcer = textCtrl;
						gridSizer->Add(textCtrl, wxGBPosition(currRow, currCol), wxGBSpan(), wxEXPAND | wxALIGN_CENTER_VERTICAL);

						currRow++;
						currCol = 2;
					}
				}
			}

			hasChar = false;
		}
		else
		{
			currRow++;
		}

		currRow++;
		currCol = 0;
	}

	sizerM->Add(gridSizer, 1, wxEXPAND | wxALL, FromDIP(20));

	text = new wxStaticText(panel, wxID_ANY, "Note 1: A slot's name can be left default if CSS/CSP/VS/Stage are not modified.");
	text->SetFont(*boldFont);
	sizerM->Add(text, 0, wxALIGN_CENTER_HORIZONTAL | wxLEFT | wxRIGHT, FromDIP(20));

	text = new wxStaticText(panel, wxID_ANY, "Note 2: You can add a new line using the '|' character.");
	text->SetFont(*boldFont);
	sizerM->Add(text, 0, wxALIGN_CENTER_HORIZONTAL | wxLEFT | wxRIGHT, FromDIP(20));

	panel->SetSizerAndFit(sizerM);
	auto sizer = new wxBoxSizer(wxVERTICAL);

	sizer->Add(panel, 1, wxEXPAND);

	wxButton* loadDefault = new wxButton(this, wxID_ANY, "Load Default");
	loadDefault->Bind(wxEVT_BUTTON, &PrcSelection::onLoadPressed, this);

	wxButton* okay = new wxButton(this, wxID_ANY, "Okay");
	okay->Bind(wxEVT_BUTTON, &PrcSelection::onClosePressed, this, wxID_ANY, wxID_ANY, new wxArgument("okay"));

	wxButton* cancel = new wxButton(this, wxID_ANY, "Cancel");
	cancel->Bind(wxEVT_BUTTON, &PrcSelection::onClosePressed, this, wxID_ANY, wxID_ANY, new wxArgument("cancel"));

	wxBoxSizer* h = new wxBoxSizer(wxHORIZONTAL);

	h->Add(loadDefault, 0);
	h->AddStretchSpacer();
	h->Add(okay, 0, wxLEFT, FromDIP(20));
	h->Add(cancel, 0, wxLEFT, FromDIP(10));

	sizer->Add(h, 0, wxEXPAND | wxALL, FromDIP(20));

	this->SetSizerAndFit(sizer);

	if (sizer->GetMinSize().y > 600)
	{
		this->SetMinSize(FromDIP(wxSize(sizer->GetMinSize().x, 600)));
	}
}

void PrcSelection::onType(wxCommandEvent& evt)
{
	wxTextCtrl* text = static_cast<prcArgument*>(evt.GetEventUserData())->text;
	if (text)
	{
		text->SetValue(evt.GetString().MakeUpper());
	}
}

void PrcSelection::onLoadPressed(wxCommandEvent& evt)
{
	for (auto i = slotNames.begin(); i != slotNames.end(); i++)
	{
		for (auto j = i->second.begin(); j != i->second.end(); j++)
		{
			auto message = mHandler->getMessage(i->first, j->first);

			j->second.cspName->SetValue(message.cspName);
			j->second.vsName->SetValue(message.vsName);

			if (
				i->first != "eflame_first"
				&& i->first != "elight_first"
				&& i->first != "pzenigame"
				&& i->first != "plizardon"
				&& i->first != "pfushigisou"
				)
			{
				j->second.stageName->SetValue(message.stageName);
			}

			if (j->first.getInt() == 0)
			{
				j->second.cssName->SetValue(message.cssName);
			}
		}
	}
}

void PrcSelection::onClosePressed(wxCommandEvent& evt)
{
	if (static_cast<wxArgument*>(evt.GetEventUserData())->str == "okay")
	{
		EndModal(wxID_OK);
	}
	else
	{
		EndModal(wxID_CANCEL);
	}
}

map<string, Slot> PrcSelection::getMaxSlots()
{
	map<string, Slot> result;

	if (!maxSlots.empty())
	{
		int index = 0;
		for (auto i = allSlots.begin(); i != allSlots.end(); i++)
		{
			if (maxSlots[index]->GetValue() != 8)
			{
				if (i->first == "eflame")
				{
					if (allSlots.find("element") == allSlots.end())
					{
						result["element"] = maxSlots[index]->GetValue();
					}
					result["eflame_first"] = maxSlots[index]->GetValue();
					result["eflame_only"] = maxSlots[index]->GetValue();
					index++;
					continue;
				}
				else if (i->first == "elight")
				{
					if (allSlots.find("element") == allSlots.end())
					{
						result["element"] = maxSlots[index]->GetValue();
					}
					result["elight_first"] = maxSlots[index]->GetValue();
					result["elight_only"] = maxSlots[index]->GetValue();
					index++;
					continue;
				}

				result[i->first] = maxSlots[index]->GetValue();
				index++;
			}
			else
			{
				index++;
			}
		}
	}

	return result;
}

map<string, map<Slot, Name>> PrcSelection::getNames()
{
	map<string, map<Slot, Name>> result;

	for (auto i = slotNames.begin(); i != slotNames.end(); i++)
	{
		for (auto j = i->second.begin(); j != i->second.end(); j++)
		{
			auto message = mHandler->getMessage(i->first, j->first);
			// TODO: Clean up big statement
			if (
				(j->first.getInt() == 0 && j->second.cssName->GetValue() != message.cssName)
				|| j->second.cspName->GetValue() != message.cspName
				|| j->second.vsName->GetValue() != message.vsName
				|| (
					i->first != "elight_first"
					&& i->first != "eflame_first"
					&& i->first != "pzenigame"
					&& i->first != "plizardon"
					&& i->first != "pfushigisou"
					&& j->second.stageName->GetValue() != message.stageName
					)
				|| j->second.announcer->GetValue() != "Default"
				|| mHandler->getName(i->first).empty()
				)
			{
				result[i->first][j->first].cspName = j->second.cspName->GetValue();
				result[i->first][j->first].vsName = j->second.vsName->GetValue();

				if (
					i->first != "elight_first"
					&& i->first != "eflame_first"
					&& i->first != "pzenigame"
					&& i->first != "plizardon"
					&& i->first != "pfushigisou"
					)
				{
					result[i->first][j->first].stageName = j->second.stageName->GetValue();
				}

				if (j->first.getInt() == 0)
				{
					result[i->first][j->first].cssName = j->second.cssName->GetValue();
				}
			}
		}
	}

	return result;
}

map<string, map<Slot, int>> PrcSelection::getDB(string type)
{
	map<string, map<Slot, int>> result;

	for (auto i = slotNames.begin(); i != slotNames.end(); i++)
	{
		for (auto j = i->second.begin(); j != i->second.end(); j++)
		{
			string code;
			Slot baseSlot;
			if (!mHandler->getName(i->first).empty())
			{
				code = i->first;
				baseSlot = mHandler->getBaseSlot(code, j->first);
			}
			else
			{
				code = mHandler->getRedirectCode(i->first);
				baseSlot = mHandler->getBaseSlot(code, mHandler->getRedirectSlot(i->first, j->first));
			}

			auto dbInit = mHandler->getXMLData(code, j->first);
			auto dbFinal = mHandler->getXMLData(code, baseSlot.getInt());

			if (type == "cGroup")
			{
				if (j->first.getInt() > 7)
				{
					if (dbFinal.cGroup != 0)
					{
						result[i->first][j->first] = dbFinal.cGroup;
					}
				}
				else if (dbInit.cGroup != dbFinal.cGroup || mHandler->getName(i->first).empty())
				{
					result[i->first][j->first] = dbFinal.cGroup;
				}
			}
			else if (type == "cIndex")
			{
				if (j->first.getInt() > 7)
				{
					if (dbFinal.cIndex != 0)
					{
						result[i->first][j->first] = dbFinal.cIndex;
					}
				}
				else if (dbInit.cIndex != dbFinal.cIndex || mHandler->getName(i->first).empty())
				{
					result[i->first][j->first] = dbFinal.cIndex;
				}
			}
			else if (type == "nIndex")
			{
				auto message = mHandler->getMessage(code, baseSlot);

				if (
					(j->first.getInt() == 0 && j->second.cssName->GetValue() != message.cssName)
					|| j->second.cspName->GetValue() != message.cspName
					|| j->second.vsName->GetValue() != message.vsName
					|| (
						i->first != "elight_first"
						&& i->first != "eflame_first"
						&& i->first != "pzenigame"
						&& i->first != "plizardon"
						&& i->first != "pfushigisou"
						&& j->second.stageName->GetValue() != message.stageName
						)
					|| j->second.announcer->GetValue() != "Default"
					|| mHandler->getName(i->first).empty()
					)
				{
					result[i->first][j->first] = j->first.getInt() + 8;
				}
				else
				{
					if (j->first.getInt() > 7)
					{
						result[i->first][j->first] = dbFinal.nIndex;
					}
					else if (dbInit.nIndex != dbFinal.nIndex)
					{
						result[i->first][j->first] = dbFinal.nIndex;
					}
				}
			}
		}
	}

	return result;
}

map<string, map<int, Name>> PrcSelection::getAnnouncers()
{
	map<string, map<int, Name>> result;

	for (auto i = slotNames.begin(); i != slotNames.end(); i++)
	{
		for (auto j = i->second.begin(); j != i->second.end(); j++)
		{
			string code;
			Slot baseSlot;
			if (!mHandler->getName(i->first).empty())
			{
				code = i->first;
				baseSlot = mHandler->getBaseSlot(code, j->first);
			}
			else
			{
				code = mHandler->getRedirectCode(i->first);
				baseSlot = mHandler->getBaseSlot(code, mHandler->getRedirectSlot(i->first, j->first));
			}

			if (baseSlot.getInt() == -1 && (i->first == "elight_only" || i->first == "elight_first"))
			{
				baseSlot = mHandler->getBaseSlot("elight", j->first);
			}

			if (baseSlot.getInt() == -1 && (i->first == "eflame_only" || i->first == "eflame_first"))
			{
				baseSlot = mHandler->getBaseSlot("eflame", j->first);
			}

			if (baseSlot.getInt() != -1)
			{
				auto message = mHandler->getMessage(code, j->first);
				auto db = mHandler->getXMLData(code, baseSlot);

				if (
					(j->first.getInt() == 0 && j->second.cssName->GetValue() != message.cssName)
					|| j->second.cspName->GetValue() != message.cspName
					|| j->second.vsName->GetValue() != message.vsName
					|| (
						i->first != "elight_first"
						&& i->first != "eflame_first"
						&& i->first != "pzenigame"
						&& i->first != "plizardon"
						&& i->first != "pfushigisou"
						&& j->second.stageName->GetValue() != message.stageName
						)
					|| j->second.announcer->GetValue() != "Default"
					|| mHandler->getName(i->first).empty()
					)
				{
					if (j->second.announcer->GetValue() == "Default")
					{
						result[i->first][j->first.getInt() + 8].announcer = db.label;
						if (!db.article.empty())
						{
							result[i->first][j->first.getInt() + 8].article = db.article;
						}
					}
					else
					{
						result[i->first][j->first.getInt() + 8].announcer = j->second.announcer->GetValue();
						if (!db.article.empty())
						{
							result[i->first][j->first.getInt() + 8].article = j->second.announcer->GetValue();
						}
					}
				}
			}
		}
	}

	return result;
}
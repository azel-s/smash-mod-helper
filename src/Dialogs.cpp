#include "Dialogs.h"
#include <wx/gbsizer.h>

BaseSlotsDialog::BaseSlotsDialog(wxWindow* parent, wxWindowID id,
    const wxString& title,
    map<string, set<string>>& additionalSlots,
    unordered_map<string, string>& charNames,
    const wxPoint& pos,
    const wxSize& size,
    long style,
    const wxString& name) :
    wxDialog(parent, id, title, pos, size, style, name)
{
    wxArrayString slotList;
    slotList.Add("00");
    slotList.Add("01");
    slotList.Add("02");
    slotList.Add("03");
    slotList.Add("04");
    slotList.Add("05");
    slotList.Add("06");
    slotList.Add("07");

    wxBoxSizer* sizerM = new wxBoxSizer(wxVERTICAL);

    // Max number of listboxes for a character
    int maxSlotBoxes = 0;
    for (auto i = additionalSlots.begin(); i != additionalSlots.end(); i++)
    {
        if (i->second.size() > maxSlotBoxes)
        {
            if (i->second.size() / 5 == 0)
            {
                maxSlotBoxes = i->second.size() % 5;
            }
            else
            {
                maxSlotBoxes = 5;
            }
        }
    }

    // Create Dialog
    for (auto i = additionalSlots.begin(); i != additionalSlots.end(); i++)
    {
        wxBoxSizer* sizerA = new wxBoxSizer(wxHORIZONTAL);
        wxBoxSizer* sizerA1 = new wxBoxSizer(wxVERTICAL);
        wxBoxSizer* sizerA2 = new wxBoxSizer(wxVERTICAL);

        int proportion = i->second.size() / 5;
        if (proportion == 0)
        {
            proportion = 1;
        }

        sizerM->AddSpacer(20);

        sizerM->Add(sizerA, proportion, wxEXPAND | wxLEFT | wxRIGHT, 20);

        wxStaticText* charName = new wxStaticText(this, wxID_ANY, charNames[i->first]);
        sizerA1->Add(charName, proportion, wxALIGN_CENTER_HORIZONTAL);
        sizerA->Add(sizerA1, 1, wxALIGN_CENTER_VERTICAL | wxRIGHT, 20);

        wxBoxSizer* sizerA2A = nullptr;
        int index = 0;
        for (auto j = i->second.begin(); j != i->second.end(); j++)
        {
            // Create new section after every 5 boxes
            if (index % 5 == 0)
            {
                sizerA2A = new wxBoxSizer(wxHORIZONTAL);
                sizerA2->Add(sizerA2A, 1);
            }

            wxStaticText* slot = new wxStaticText(this, wxID_ANY, "Slot " + *j + ": ", wxDefaultPosition, wxSize(45, -1));
            sizerA2A->Add(slot, 0, wxALIGN_CENTER_VERTICAL);

            wxChoice* baseSlotList = new wxChoice(this, wxID_ANY, wxDefaultPosition, wxSize(40, -1), slotList);
            baseSlotList->Select(0);
            baseSlots.push_back(baseSlotList);
            sizerA2A->Add(baseSlotList, 1, wxALIGN_CENTER_VERTICAL | wxRIGHT, 10);

            index++;
        }

        sizerA->Add(sizerA2, maxSlotBoxes - 1, wxALIGN_CENTER_VERTICAL);
    }

    sizerM->Add(CreateButtonSizer(wxOK | wxCANCEL), 0, wxALIGN_RIGHT | wxTOP | wxRIGHT | wxBOTTOM, 20);

    this->SetSizerAndFit(sizerM);
}

map<string, map<string, set<string>>> BaseSlotsDialog::getBaseSlots(map<string, set<string>>& additionalSlots)
{
    int index = 0;

    for (auto i = additionalSlots.begin(); i != additionalSlots.end(); i++)
    {
        for (auto j = i->second.begin(); j != i->second.end(); j++)
        {
            slots[i->first][baseSlots[index]->GetStringSelection().ToStdString()].insert(*j);
            index++;
        }
    }

    return slots;
}

prcxDialog::prcxDialog(wxWindow* parent, wxWindowID id,
    const wxString& title,
    map<string, set<string>>& allSlots,
    unordered_map<string, string>& charNames,
    bool slots,
    bool names,
    const wxPoint& pos,
    const wxSize& size,
    long style,
    const wxString& name) :
    wxDialog(parent, id, title, pos, size, style, name)
{
    wxBoxSizer* sizerM = new wxBoxSizer(wxVERTICAL);
    wxGridBagSizer* gridSizer = new wxGridBagSizer(5, 20);
    wxTextCtrl* textCtrl = new wxTextCtrl();
    wxStaticText* text;

    wxFont* boldFont = new wxFont();
    boldFont->SetWeight(wxFONTWEIGHT_BOLD);

    gridSizer->SetFlexibleDirection(wxBOTH);

    // Create Header
    text = new wxStaticText(this, wxID_ANY, "Characters");
    text->SetFont(*boldFont);
    gridSizer->Add(text, wxGBPosition(0, 0));


    // Check if slot 00 exists anywhere
    bool hasSlot00 = false;
    for (auto i = allSlots.begin(); i != allSlots.end(); i++)
    {
        if (i->second.find("00") != i->second.end())
        {
            hasSlot00 = true;
            break;
        }
    }

    if (slots)
    {
        text = new wxStaticText(this, wxID_ANY, "Max Slots");
        text->SetFont(*boldFont);
        gridSizer->Add(text, wxGBPosition(0, 1), wxGBSpan(), wxALIGN_CENTER_HORIZONTAL);
    }

    if (names)
    {
        int currCol = 2;

        text = new wxStaticText(this, wxID_ANY, "Slot");
        text->SetFont(*boldFont);
        gridSizer->Add(text, wxGBPosition(0, currCol), wxGBSpan(), wxALIGN_CENTER_HORIZONTAL);
        currCol++;

        if (hasSlot00)
        {
            text = new wxStaticText(this, wxID_ANY, "CSS Name");
            text->SetFont(*boldFont);
            gridSizer->Add(text, wxGBPosition(0, currCol), wxGBSpan(), wxALIGN_CENTER_HORIZONTAL);
            currCol++;
        }

        text = new wxStaticText(this, wxID_ANY, "CSP Name");
        text->SetFont(*boldFont);
        gridSizer->Add(text, wxGBPosition(0, currCol), wxGBSpan(), wxALIGN_CENTER_HORIZONTAL);
        currCol++;

        text = new wxStaticText(this, wxID_ANY, "VS/Results Name");
        text->SetFont(*boldFont);
        gridSizer->Add(text, wxGBPosition(0, currCol), wxGBSpan(), wxALIGN_CENTER_HORIZONTAL);
        currCol++;

        text = new wxStaticText(this, wxID_ANY, "Boxing Ring Name");
        text->SetFont(*boldFont);
        gridSizer->Add(text, wxGBPosition(0, currCol), wxGBSpan(), wxALIGN_CENTER_HORIZONTAL);
        currCol++;

        text = new wxStaticText(this, wxID_ANY, "Custom Announcer");
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
    }

    int currRow = 1;
    int currCol = 0;

    // Create fields
    for (auto i = allSlots.begin(); i != allSlots.end(); i++)
    {
        text = new wxStaticText(this, wxID_ANY, charNames[i->first]);

        if (names)
        {
            gridSizer->Add(text, wxGBPosition(currRow, currCol), wxGBSpan(i->second.size(), 1), wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL);
        }
        else
        {
            gridSizer->Add(text, wxGBPosition(currRow, currCol), wxGBSpan(), wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL);
        }
        currCol++;

        if (slots)
        {
            wxSpinCtrl* userSlot = new wxSpinCtrl(this, wxID_ANY, "", wxDefaultPosition, wxSize(50, -1), wxSP_WRAP, 8, 255, stoi((*(i->second.rbegin()))) + 1);
            maxSlots.push_back(userSlot);

            if (names)
            {
                gridSizer->Add(userSlot, wxGBPosition(currRow, currCol), wxGBSpan(i->second.size(), 1), wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL);
            }
            else
            {
                gridSizer->Add(userSlot, wxGBPosition(currRow, currCol), wxGBSpan(), wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL);
            }
        }
        currCol++;

        if (names && i->first != "element")
        {
            for (auto j = i->second.begin(); j != i->second.end(); j++)
            {
                text = new wxStaticText(this, wxID_ANY, "c" + *j);
                gridSizer->Add(text, wxGBPosition(currRow, currCol), wxGBSpan(), wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL);
                currCol++;

                string charcode = i->first;

                if (i->first == "eflame")
                {
                    charcode = "eflame_only";
                }
                else if (i->first == "elight")
                {
                    charcode = "elight_only";
                }

                if (hasSlot00 && *j == "00")
                {
                    textCtrl = new wxTextCtrl(this, wxID_ANY, "Name", wxDefaultPosition, wxDefaultSize, wxTE_CENTRE);
                    slotNames[charcode][*j].cssName = textCtrl;
                    gridSizer->Add(textCtrl, wxGBPosition(currRow, currCol), wxGBSpan(), wxEXPAND | wxALIGN_CENTER_VERTICAL);
                    currCol++;
                }
                else if (hasSlot00)
                {
                    currCol++;
                }

                textCtrl = new wxTextCtrl(this, wxID_ANY, "Name", wxDefaultPosition, wxDefaultSize, wxTE_CENTRE);
                slotNames[charcode][*j].cspName = textCtrl;
                gridSizer->Add(textCtrl, wxGBPosition(currRow, currCol), wxGBSpan(), wxEXPAND | wxALIGN_CENTER_VERTICAL);
                currCol++;

                textCtrl = new wxTextCtrl(this, wxID_ANY, "NAME", wxDefaultPosition, wxDefaultSize, wxTE_CENTRE);
                slotNames[charcode][*j].vsName = textCtrl;
                gridSizer->Add(textCtrl, wxGBPosition(currRow, currCol), wxGBSpan(), wxEXPAND | wxALIGN_CENTER_VERTICAL);
                currCol++;

                textCtrl = new wxTextCtrl(this, wxID_ANY, "Stage Name", wxDefaultPosition, wxDefaultSize, wxTE_CENTRE);
                slotNames[charcode][*j].stageName = textCtrl;
                gridSizer->Add(textCtrl, wxGBPosition(currRow, currCol), wxGBSpan(), wxEXPAND | wxALIGN_CENTER_VERTICAL);
                currCol++;

                textCtrl = new wxTextCtrl(this, wxID_ANY, "Default", wxDefaultPosition, wxDefaultSize, wxTE_CENTRE);
                slotNames[charcode][*j].announcer = textCtrl;
                gridSizer->Add(textCtrl, wxGBPosition(currRow, currCol), wxGBSpan(), wxEXPAND | wxALIGN_CENTER_VERTICAL);
                currCol++;

                currRow++;
                currCol = 2;
            }

            // Pyra and Mythra both have same slots
            if (i->first == "elight" && allSlots.find("eflame") != allSlots.end())
            {
                currRow++;
                currCol = 0;

                // Get slot union
                set<string> slotUnion;
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
                    text = new wxStaticText(this, wxID_ANY, "Pyra/Mythra");
                    gridSizer->Add(text, wxGBPosition(currRow, currCol), wxGBSpan(slotUnion.size() * 2, 1), wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL);
                    currCol++;

                    text = new wxStaticText(this, wxID_ANY, "Unique");
                    gridSizer->Add(text, wxGBPosition(currRow, currCol), wxGBSpan(slotUnion.size() * 2, 1), wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL);
                    currCol++;

                    for (auto j = slotUnion.begin(); j != slotUnion.end(); j++)
                    {
                        text = new wxStaticText(this, wxID_ANY, "c" + *j + " P/M");
                        gridSizer->Add(text, wxGBPosition(currRow, currCol), wxGBSpan(), wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL);
                        currCol++;

                        if (hasSlot00 && *j == "00")
                        {
                            textCtrl = new wxTextCtrl(this, wxID_ANY, "Name", wxDefaultPosition, wxDefaultSize, wxTE_CENTRE);
                            slotNames["eflame_first"][*j].cssName = textCtrl;
                            gridSizer->Add(textCtrl, wxGBPosition(currRow, currCol), wxGBSpan(), wxEXPAND | wxALIGN_CENTER_VERTICAL);
                            currCol++;
                        }
                        else if (hasSlot00)
                        {
                            currCol++;
                        }

                        textCtrl = new wxTextCtrl(this, wxID_ANY, "Name", wxDefaultPosition, wxDefaultSize, wxTE_CENTRE);
                        slotNames["eflame_first"][*j].cspName = textCtrl;
                        gridSizer->Add(textCtrl, wxGBPosition(currRow, currCol), wxGBSpan(), wxEXPAND | wxALIGN_CENTER_VERTICAL);
                        currCol++;

                        textCtrl = new wxTextCtrl(this, wxID_ANY, "NAME", wxDefaultPosition, wxDefaultSize, wxTE_CENTRE);
                        slotNames["eflame_first"][*j].vsName = textCtrl;
                        gridSizer->Add(textCtrl, wxGBPosition(currRow, currCol), wxGBSpan(), wxEXPAND | wxALIGN_CENTER_VERTICAL);
                        currCol += 2;

                        textCtrl = new wxTextCtrl(this, wxID_ANY, "Default", wxDefaultPosition, wxDefaultSize, wxTE_CENTRE);
                        slotNames["eflame_first"][*j].announcer = textCtrl;
                        gridSizer->Add(textCtrl, wxGBPosition(currRow, currCol), wxGBSpan(), wxEXPAND | wxALIGN_CENTER_VERTICAL);

                        currRow++;
                        currCol = 2;

                        text = new wxStaticText(this, wxID_ANY, "c" + *j + " M/P");
                        gridSizer->Add(text, wxGBPosition(currRow, currCol), wxGBSpan(), wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL);
                        currCol++;

                        if (hasSlot00 && *j == "00")
                        {
                            textCtrl = new wxTextCtrl(this, wxID_ANY, "Name", wxDefaultPosition, wxDefaultSize, wxTE_CENTRE);
                            slotNames["elight_first"][*j].cssName = textCtrl;
                            gridSizer->Add(textCtrl, wxGBPosition(currRow, currCol), wxGBSpan(), wxEXPAND | wxALIGN_CENTER_VERTICAL);
                            currCol++;
                        }
                        else if (hasSlot00)
                        {
                            currCol++;
                        }

                        textCtrl = new wxTextCtrl(this, wxID_ANY, "Name", wxDefaultPosition, wxDefaultSize, wxTE_CENTRE);
                        slotNames["elight_first"][*j].cspName = textCtrl;
                        gridSizer->Add(textCtrl, wxGBPosition(currRow, currCol), wxGBSpan(), wxEXPAND | wxALIGN_CENTER_VERTICAL);
                        currCol++;

                        textCtrl = new wxTextCtrl(this, wxID_ANY, "NAME", wxDefaultPosition, wxDefaultSize, wxTE_CENTRE);
                        slotNames["elight_first"][*j].vsName = textCtrl;
                        gridSizer->Add(textCtrl, wxGBPosition(currRow, currCol), wxGBSpan(), wxEXPAND | wxALIGN_CENTER_VERTICAL);
                        currCol += 2;

                        textCtrl = new wxTextCtrl(this, wxID_ANY, "Default", wxDefaultPosition, wxDefaultSize, wxTE_CENTRE);
                        slotNames["elight_first"][*j].announcer = textCtrl;
                        gridSizer->Add(textCtrl, wxGBPosition(currRow, currCol), wxGBSpan(), wxEXPAND | wxALIGN_CENTER_VERTICAL);

                        currRow++;
                        currCol = 2;
                    }
                }
            }
        }

        currRow++;
        currCol = 0;
    }

    sizerM->Add(gridSizer, 1, wxEXPAND | wxLEFT | wxRIGHT | wxTOP, 20);
    sizerM->Add(CreateButtonSizer(wxOK | wxCANCEL), 0, wxALIGN_RIGHT | wxALL, 20);

    this->SetSizerAndFit(sizerM);
}

map<string, int> prcxDialog::getMaxSlots(map<string, set<string>>& allSlots)
{
    map<string, int> result;

    if (!maxSlots.empty())
    {
        int index = 0;
        for (auto i = allSlots.begin(); i != allSlots.end(); i++)
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
                result["elight_first"] = maxSlots[index]->GetValue();
                result["elight_only"] = maxSlots[index]->GetValue();
                index++;
                continue;
            }

            result[i->first] = maxSlots[index]->GetValue();
            index++;
        }
    }

    return result;
}

map<string, map<int, Name>> prcxDialog::getNames()
{
    map<string, map<int, Name>> result;

    int index = 0;
    for (auto i = slotNames.begin(); i != slotNames.end(); i++)
    {
        for (auto j = i->second.begin(); j != i->second.end(); j++)
        {
            if (j->second.cspName->GetValue() != "Name" || j->second.vsName->GetValue() != "NAME" || (i->first != "elight_first" && i->first != "eflame_first" && j->second.stageName->GetValue() != "Stage Name"))
            {
                result[i->first][stoi(j->first)].cspName = j->second.cspName->GetValue();
                result[i->first][stoi(j->first)].vsName = j->second.vsName->GetValue();

                if (i->first != "eflame_first" && i->first != "elight_first")
                {
                    result[i->first][stoi(j->first)].stageName = j->second.stageName->GetValue();
                }

                if (stoi(j->first) == 0)
                {
                    result[i->first][stoi(j->first)].cssName = j->second.cssName->GetValue();
                }
            }
        }
    }

    return result;
}

map<string, map<int, string>> prcxDialog::getAnnouncers()
{
    map<string, map<int, string>> result;

    int index = 0;
    for (auto i = slotNames.begin(); i != slotNames.end(); i++)
    {
        for (auto j = i->second.begin(); j != i->second.end(); j++)
        {
            if (j->second.cspName->GetValue() != "Name" || j->second.vsName->GetValue() != "NAME" || (i->first != "elight_first" && i->first != "eflame_first" && j->second.stageName->GetValue() != "Stage Name") || j->second.announcer->GetValue() != "Default" || stoi(j->first) > 7)
            {
                result[i->first][stoi(j->first)] = j->second.announcer->GetValue();
            }
        }
    }

    return result;
}

InklingDialog::InklingDialog(wxWindow* parent, wxWindowID id,
    const wxString& title,
    map<string, string>& slotsMap,
    map<int, InklingColor>& inklingColors,
    unordered_map<string, string>& charNames,
    const wxPoint& pos,
    const wxSize& size,
    long style,
    const wxString& name) :
    wxDialog(parent, id, title, pos, size, style, name)
{
    this->slotsMap = slotsMap;

    list.Add("00");
    list.Add("01");
    list.Add("02");
    list.Add("03");
    list.Add("04");
    list.Add("05");
    list.Add("06");
    list.Add("07");

    for (auto i = slotsMap.begin(); i != slotsMap.end(); i++)
    {
        int slot = stoi(i->first);

        if (slot > 7)
        {
            if (slot < 10)
            {
                list.Add("0" + to_string(slot));
            }
            else
            {
                list.Add(to_string(slot));
            }
        }
    }

    sizerM = new wxBoxSizer(wxVERTICAL);
    sizerV = nullptr;
    sizerH = nullptr;

    wxFont* boldFont = new wxFont();
    boldFont->SetWeight(wxFONTWEIGHT_BOLD);

    // Create Header
    wxStaticText* header = new wxStaticText(this, wxID_ANY, "General Effect Color | Roller Color | Slot Number");
    header->SetFont(*boldFont);

    sizerV = new wxBoxSizer(wxVERTICAL);
    sizerV->Add(header, 0, wxALIGN_CENTER_HORIZONTAL);
    sizerM->Add(sizerV, 0, wxALIGN_CENTER_HORIZONTAL | wxLEFT | wxTOP | wxRIGHT, 20);

    int index = 0;

    for (auto i = inklingColors.begin(); i != inklingColors.end(); i++)
    {
        // Create new section after every 4 boxes
        if (index % 3 == 0)
        {
            sizerH = new wxBoxSizer(wxHORIZONTAL);
            sizerM->Add(sizerH, 1, wxALIGN_CENTER_HORIZONTAL | wxLEFT | wxTOP | wxRIGHT, 20);
        }

        sizerV = new wxBoxSizer(wxVERTICAL);
        sizerH->Add(sizerV, 1, wxALIGN_CENTER_VERTICAL);

        wxColourPickerCtrl* effectPicker = new wxColourPickerCtrl(this, wxID_ANY, wxStockGDI::COLOUR_BLUE, wxDefaultPosition, wxDefaultSize);
        sizerV->Add(effectPicker, 1, wxALIGN_CENTER_HORIZONTAL);
        effectPicker->SetColour(i->second.effect);
        finalEffects.push_back(effectPicker);

        wxColourPickerCtrl* arrowPicker = new wxColourPickerCtrl(this, wxID_ANY, wxStockGDI::COLOUR_BLUE, wxDefaultPosition, wxDefaultSize);
        sizerV->Add(arrowPicker, 1, wxALIGN_CENTER_HORIZONTAL);
        arrowPicker->SetColour(i->second.arrow);
        finalArrows.push_back(arrowPicker);

        wxChoice* slotList = new wxChoice(this, wxID_ANY, wxDefaultPosition, wxSize(40, -1), list);
        sizerV->Add(slotList, 1, wxALIGN_CENTER_HORIZONTAL);
        finalSlots.push_back(slotList);

        if (i->first > 9)
        {
            slotList->SetStringSelection(to_string(i->first));
        }
        else
        {
            slotList->SetStringSelection("0" + to_string(i->first));
        }

        index++;
    }

    // Add Button
    add.sizer = new wxBoxSizer(wxVERTICAL);
    add.button = new wxButton(this, wxID_ANY, "+", wxDefaultPosition, wxSize(40, -1));

    add.sizer->Add(add.button, 1, wxALIGN_CENTER_HORIZONTAL);
    add.button->Bind(wxEVT_BUTTON, &InklingDialog::onAddPressed, this);

    add.list = new wxChoice(this, wxID_ANY, wxDefaultPosition, wxSize(40, -1), list);
    add.list->Select(0);
    add.sizer->Add(add.list, 1, wxALIGN_CENTER_HORIZONTAL);

    if (sizerH == nullptr || sizerH->GetItemCount() == 3)
    {
        sizerH = new wxBoxSizer(wxHORIZONTAL);
        sizerH->Add(add.sizer, 1, wxALIGN_CENTER_VERTICAL);

        sizerM->Add(sizerH, 1, wxALIGN_CENTER_HORIZONTAL | wxLEFT | wxTOP | wxRIGHT, 20);
    }
    else
    {
        sizerH->Add(add.sizer, 1, wxALIGN_CENTER_VERTICAL | wxLEFT, 20);
    }

    sizerM->Add(CreateButtonSizer(wxOK | wxCANCEL), 0, wxALIGN_RIGHT | wxALL, 20);
    this->SetSizerAndFit(sizerM);

    this->SetClientSize(350, -1);
    this->SetMinSize(wxSize(350, -1));
}


void InklingDialog::onAddPressed(wxCommandEvent& evt)
{
    InklingColor color;
    string slot = add.list->GetStringSelection().ToStdString();

    if (stoi(slot) > 7)
    {
        color = vanillaColors[stoi(slotsMap[slot])];
    }
    else
    {
        color = vanillaColors[stoi(slot)];
    }

    sizerV = new wxBoxSizer(wxVERTICAL);

    wxColourPickerCtrl* effectPicker = new wxColourPickerCtrl(this, wxID_ANY, wxStockGDI::COLOUR_BLUE, wxDefaultPosition, wxDefaultSize);
    effectPicker->SetColour(color.effect);
    sizerV->Add(effectPicker, 1, wxALIGN_CENTER_HORIZONTAL);
    finalEffects.push_back(effectPicker);

    wxColourPickerCtrl* arrowPicker = new wxColourPickerCtrl(this, wxID_ANY, wxStockGDI::COLOUR_BLUE, wxDefaultPosition, wxDefaultSize);
    arrowPicker->SetColour(color.arrow);
    sizerV->Add(arrowPicker, 1, wxALIGN_CENTER_HORIZONTAL);
    finalArrows.push_back(arrowPicker);

    wxChoice* slotList = new wxChoice(this, wxID_ANY, wxDefaultPosition, wxSize(40, -1), list);
    slotList->SetSelection(add.list->GetSelection());
    sizerV->Add(slotList, 1, wxALIGN_CENTER_HORIZONTAL);
    finalSlots.push_back(slotList);

    sizerH->Insert(sizerH->GetItemCount() - 1, sizerV, 1, wxALIGN_CENTER_VERTICAL);

    if (sizerH->GetItemCount() == 4)
    {
        sizerH->Detach(add.sizer);

        sizerH = new wxBoxSizer(wxHORIZONTAL);
        sizerH->Add(add.sizer, 1, wxALIGN_CENTER_VERTICAL);

        sizerM->Insert(sizerM->GetItemCount() - 1, sizerH, 1, wxALIGN_CENTER_HORIZONTAL | wxLEFT | wxTOP | wxRIGHT, 20);
    }

    SendSizeEvent();

    // Adjust Size for best fit
    if (this->GetSize().x < this->GetBestSize().x)
    {
        this->SetSize(wxSize(this->GetBestSize().x, this->GetSize().y));
    }
    if (this->GetSize().y < this->GetBestSize().y)
    {
        this->SetSize(wxSize(this->GetSize().x, this->GetBestSize().y));
    }
}

map<int, InklingColor> InklingDialog::getFinalColors()
{
    map<int, InklingColor> finalColors;

    for (int i = 0; i < finalSlots.size(); i++)
    {
        finalColors[stoi(finalSlots[i]->GetStringSelection().ToStdString())].effect = finalEffects[i]->GetColour();
        finalColors[stoi(finalSlots[i]->GetStringSelection().ToStdString())].arrow = finalArrows[i]->GetColour();
    }

    return finalColors;
}

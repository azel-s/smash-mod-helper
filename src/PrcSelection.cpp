#include "PrcSelection.h"
#include <wx/gbsizer.h>

PrcSelection::PrcSelection(wxWindow* parent, wxWindowID id,
    const wxString& title,
    SmashData& mod,
    bool readPrevNames,
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

    auto allSlots = mod.getAllSlots();
    auto prevNames = mod.readNames();
    bool slots = mod.hasAdditionalSlot();
    
    string fieldName;

    bool hasChar = false;
    bool hasSlot = false;

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

    if (true)
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
        if (readPrevNames && prevNames.find(i->first) != prevNames.end())
        {
            hasChar = true;
        }

        text = new wxStaticText(this, wxID_ANY, mod.charNames[i->first]);

        if (true)
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

            if (true)
            {
                gridSizer->Add(userSlot, wxGBPosition(currRow, currCol), wxGBSpan(i->second.size(), 1), wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL);
            }
            else
            {
                gridSizer->Add(userSlot, wxGBPosition(currRow, currCol), wxGBSpan(), wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL);
            }
        }
        currCol++;

        if (true && i->first != "element")
        {
            for (auto j = i->second.begin(); j != i->second.end(); j++)
            {
                if (hasChar && prevNames[i->first].find(*j) != prevNames[i->first].end())
                {
                    hasSlot = true;
                }

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
                    fieldName = hasSlot ? prevNames[i->first][*j].cssName : "Name";

                    textCtrl = new wxTextCtrl(this, wxID_ANY, name, wxDefaultPosition, wxDefaultSize, wxTE_CENTRE);
                    slotNames[charcode][*j].cssName = textCtrl;
                    gridSizer->Add(textCtrl, wxGBPosition(currRow, currCol), wxGBSpan(), wxEXPAND | wxALIGN_CENTER_VERTICAL);
                    currCol++;
                }
                else if (hasSlot00)
                {
                    currCol++;
                }

                fieldName = hasSlot ? prevNames[i->first][*j].cspName : "Name";
                textCtrl = new wxTextCtrl(this, wxID_ANY, fieldName, wxDefaultPosition, wxDefaultSize, wxTE_CENTRE);
                slotNames[charcode][*j].cspName = textCtrl;
                gridSizer->Add(textCtrl, wxGBPosition(currRow, currCol), wxGBSpan(), wxEXPAND | wxALIGN_CENTER_VERTICAL);
                currCol++;

                fieldName = hasSlot ? prevNames[i->first][*j].vsName : "NAME";
                textCtrl = new wxTextCtrl(this, wxID_ANY, fieldName, wxDefaultPosition, wxDefaultSize, wxTE_CENTRE);
                slotNames[charcode][*j].vsName = textCtrl;
                gridSizer->Add(textCtrl, wxGBPosition(currRow, currCol), wxGBSpan(), wxEXPAND | wxALIGN_CENTER_VERTICAL);
                currCol++;

                fieldName = hasSlot ? prevNames[i->first][*j].stageName : "Stage Name";
                textCtrl = new wxTextCtrl(this, wxID_ANY, fieldName, wxDefaultPosition, wxDefaultSize, wxTE_CENTRE);
                slotNames[charcode][*j].stageName = textCtrl;
                gridSizer->Add(textCtrl, wxGBPosition(currRow, currCol), wxGBSpan(), wxEXPAND | wxALIGN_CENTER_VERTICAL);
                currCol++;

                textCtrl = new wxTextCtrl(this, wxID_ANY, "Default", wxDefaultPosition, wxDefaultSize, wxTE_CENTRE);
                slotNames[charcode][*j].announcer = textCtrl;
                gridSizer->Add(textCtrl, wxGBPosition(currRow, currCol), wxGBSpan(), wxEXPAND | wxALIGN_CENTER_VERTICAL);
                currCol++;

                currRow++;
                currCol = 2;

                hasSlot = true;
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
                    bool hasFlame = false;
                    bool hasLight = false;
                    if (readPrevNames && prevNames.find("eflame_first") != prevNames.end())
                    {
                        hasFlame = true;
                    }
                    if (hasChar && prevNames.find("elight_first") != prevNames.end())
                    {
                        hasLight = true;
                    }

                    text = new wxStaticText(this, wxID_ANY, "Pyra/Mythra");
                    gridSizer->Add(text, wxGBPosition(currRow, currCol), wxGBSpan(slotUnion.size() * 2, 1), wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL);
                    currCol++;

                    text = new wxStaticText(this, wxID_ANY, "Unique");
                    gridSizer->Add(text, wxGBPosition(currRow, currCol), wxGBSpan(slotUnion.size() * 2, 1), wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL);
                    currCol++;

                    for (auto j = slotUnion.begin(); j != slotUnion.end(); j++)
                    {
                        bool hasFlameSlot = false;

                        if (hasFlame && prevNames["eflame_first"].find(*j) != prevNames["eflame_first"].end())
                        {
                            hasFlameSlot = true;
                        }

                        text = new wxStaticText(this, wxID_ANY, "c" + *j + " P/M");
                        gridSizer->Add(text, wxGBPosition(currRow, currCol), wxGBSpan(), wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL);
                        currCol++;

                        if (hasSlot00 && *j == "00")
                        {
                            fieldName = hasFlameSlot ? prevNames[i->first][*j].cssName : "Name";
                            textCtrl = new wxTextCtrl(this, wxID_ANY, fieldName, wxDefaultPosition, wxDefaultSize, wxTE_CENTRE);
                            slotNames["eflame_first"][*j].cssName = textCtrl;
                            gridSizer->Add(textCtrl, wxGBPosition(currRow, currCol), wxGBSpan(), wxEXPAND | wxALIGN_CENTER_VERTICAL);
                            currCol++;
                        }
                        else if (hasSlot00)
                        {
                            currCol++;
                        }

                        fieldName = hasFlameSlot ? prevNames[i->first][*j].cspName : "Name";
                        textCtrl = new wxTextCtrl(this, wxID_ANY, fieldName, wxDefaultPosition, wxDefaultSize, wxTE_CENTRE);
                        slotNames["eflame_first"][*j].cspName = textCtrl;
                        gridSizer->Add(textCtrl, wxGBPosition(currRow, currCol), wxGBSpan(), wxEXPAND | wxALIGN_CENTER_VERTICAL);
                        currCol++;

                        fieldName = hasFlameSlot ? prevNames[i->first][*j].vsName : "NAME";
                        textCtrl = new wxTextCtrl(this, wxID_ANY, fieldName, wxDefaultPosition, wxDefaultSize, wxTE_CENTRE);
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

                        bool hasLightSlot = false;

                        if (hasLight && prevNames["elight_first"].find(*j) != prevNames["elight_first"].end())
                        {
                            hasLightSlot = true;
                        }

                        if (hasSlot00 && *j == "00")
                        {
                            fieldName = hasLightSlot ? prevNames[i->first][*j].cssName : "Name";
                            textCtrl = new wxTextCtrl(this, wxID_ANY, fieldName, wxDefaultPosition, wxDefaultSize, wxTE_CENTRE);
                            slotNames["elight_first"][*j].cssName = textCtrl;
                            gridSizer->Add(textCtrl, wxGBPosition(currRow, currCol), wxGBSpan(), wxEXPAND | wxALIGN_CENTER_VERTICAL);
                            currCol++;
                        }
                        else if (hasSlot00)
                        {
                            currCol++;
                        }

                        fieldName = hasLightSlot ? prevNames[i->first][*j].cspName : "Name";
                        textCtrl = new wxTextCtrl(this, wxID_ANY, fieldName, wxDefaultPosition, wxDefaultSize, wxTE_CENTRE);
                        slotNames["elight_first"][*j].cspName = textCtrl;
                        gridSizer->Add(textCtrl, wxGBPosition(currRow, currCol), wxGBSpan(), wxEXPAND | wxALIGN_CENTER_VERTICAL);
                        currCol++;

                        fieldName = hasLightSlot ? prevNames[i->first][*j].vsName : "NAME";
                        textCtrl = new wxTextCtrl(this, wxID_ANY, fieldName, wxDefaultPosition, wxDefaultSize, wxTE_CENTRE);
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

            hasChar = false;
        }

        currRow++;
        currCol = 0;
    }

    sizerM->Add(gridSizer, 1, wxEXPAND | wxLEFT | wxRIGHT | wxTOP, 20);
    sizerM->Add(CreateButtonSizer(wxOK | wxCANCEL), 0, wxALIGN_RIGHT | wxALL, 20);

    this->SetSizerAndFit(sizerM);
}

map<string, int> PrcSelection::getMaxSlots(map<string, set<string>>& allSlots)
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

map<string, map<int, Name>> PrcSelection::getNames()
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

map<string, map<int, string>> PrcSelection::getAnnouncers()
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
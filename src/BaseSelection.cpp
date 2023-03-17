#include "BaseSelection.h"

BaseSelection::BaseSelection(wxWindow* parent, wxWindowID id,
    const wxString& title,
    SmashData& mod,
    bool readPrevBase,
    const wxPoint& pos,
    const wxSize& size,
    long style,
    const wxString& name) :
    wxDialog(parent, id, title, pos, size, style, name)
{
    this->mod = &mod;

    wxArrayString slotList;
    slotList.Add("00");
    slotList.Add("01");
    slotList.Add("02");
    slotList.Add("03");
    slotList.Add("04");
    slotList.Add("05");
    slotList.Add("06");
    slotList.Add("07");

    auto addSlots = this->mod->getAddSlots();

    map<string, map<string, int>> prevBaseSlots;
    bool hasChar = false;
    bool hasSlot = false;

    if (readPrevBase)
    {
        prevBaseSlots = this->mod->readBaseSlots();
    }

    wxBoxSizer* sizerM = new wxBoxSizer(wxVERTICAL);

    // Max number of listboxes for a character
    int maxSlotBoxes = 0;
    for (auto i = addSlots.begin(); i != addSlots.end(); i++)
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
    for (auto i = addSlots.begin(); i != addSlots.end(); i++)
    {
        if (prevBaseSlots.find(i->first) != prevBaseSlots.end())
        {
            hasChar = true;
        }

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

        wxStaticText* charName = new wxStaticText(this, wxID_ANY, this->mod->charNames[i->first]);
        sizerA1->Add(charName, proportion, wxALIGN_CENTER_HORIZONTAL);
        sizerA->Add(sizerA1, 1, wxALIGN_CENTER_VERTICAL | wxRIGHT, 20);

        wxBoxSizer* sizerA2A = nullptr;
        int index = 0;
        for (auto j = i->second.begin(); j != i->second.end(); j++)
        {
            if (hasChar && prevBaseSlots[i->first].find(*j) != prevBaseSlots[i->first].end())
            {
                hasSlot = true;
            }

            // Create new section after every 5 boxes
            if (index % 5 == 0)
            {
                sizerA2A = new wxBoxSizer(wxHORIZONTAL);
                sizerA2->Add(sizerA2A, 1);
            }

            wxStaticText* slot = new wxStaticText(this, wxID_ANY, "Slot " + *j + ": ", wxDefaultPosition, wxSize(45, -1));
            sizerA2A->Add(slot, 0, wxALIGN_CENTER_VERTICAL);

            wxChoice* baseSlotList = new wxChoice(this, wxID_ANY, wxDefaultPosition, wxSize(40, -1), slotList);

            if (hasSlot)
            {
                baseSlotList->Select(prevBaseSlots[i->first][*j]);
            }
            else
            {
                baseSlotList->Select(0);
            }

            baseSlots.push_back(baseSlotList);
            sizerA2A->Add(baseSlotList, 1, wxALIGN_CENTER_VERTICAL | wxRIGHT, 10);

            hasSlot = false;
            index++;
        }

        sizerA->Add(sizerA2, maxSlotBoxes - 1, wxALIGN_CENTER_VERTICAL);

        hasChar = false;
    }

    sizerM->Add(CreateButtonSizer(wxOK | wxCANCEL), 0, wxALIGN_RIGHT | wxTOP | wxRIGHT | wxBOTTOM, 20);

    this->SetSizerAndFit(sizerM);
}

map<string, map<string, set<string>>> BaseSelection::getBaseSlots()
{
    auto addSlots = mod->getAddSlots();

    int index = 0;

    for (auto i = addSlots.begin(); i != addSlots.end(); i++)
    {
        for (auto j = i->second.begin(); j != i->second.end(); j++)
        {
            slots[i->first][baseSlots[index]->GetStringSelection().ToStdString()].insert(*j);
            index++;
        }
    }

    return slots;
}
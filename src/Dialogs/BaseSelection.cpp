#include "BaseSelection.h"

BaseSelection::BaseSelection(wxWindow* parent, wxWindowID id,
    const wxString& title,
    ModHandler& mHandler,
    bool readPrevBase,
    const wxPoint& pos,
    const wxSize& size,
    long style,
    const wxString& name) :
    wxDialog(parent, id, title, pos, size, style, name)
{
    this->mHandler = &mHandler;

    wxArrayString slotList;
    slotList.Add("c00");
    slotList.Add("c01");
    slotList.Add("c02");
    slotList.Add("c03");
    slotList.Add("c04");
    slotList.Add("c05");
    slotList.Add("c06");
    slotList.Add("c07");

    auto addSlots = mHandler.getAddSlots();

    map<string, map<Slot, Slot>> prevBaseSlots;
    bool hasChar = false;
    bool hasSlot = false;

    if (readPrevBase)
    {
        prevBaseSlots = mHandler.readBaseSlots();
    }

    auto panel = new wxScrolled<wxPanel>(this, wxID_ANY, wxDefaultPosition, wxDefaultSize);
    panel->SetScrollRate(0, 10);

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
        if (i->second.size() % 5 != 0)
        {
            proportion += 1;
        }

        sizerM->AddSpacer(20);

        sizerM->Add(sizerA, proportion, wxEXPAND | wxLEFT | wxRIGHT, 20);

        wxStaticText* charName = new wxStaticText(panel, wxID_ANY, mHandler.getName(i->first));
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

            wxStaticText* slot = new wxStaticText(panel, wxID_ANY, "Slot c" + j->getString() + ": ", wxDefaultPosition, wxSize(45, -1));
            sizerA2A->Add(slot, 0, wxALIGN_CENTER_VERTICAL);

            wxChoice* baseSlotList = new wxChoice(panel, wxID_ANY, wxDefaultPosition, wxSize(40, -1), slotList);

            if (hasSlot)
            {
                baseSlotList->SetStringSelection("c" + prevBaseSlots[i->first][*j].getString());
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

    panel->SetSizerAndFit(sizerM);

    auto sizer = new wxBoxSizer(wxVERTICAL);

    sizer->Add(panel, 1, wxEXPAND);
    sizer->Add(CreateButtonSizer(wxOK | wxCANCEL), 0, wxALIGN_RIGHT | wxTOP | wxRIGHT | wxBOTTOM, 20);

    this->SetSizerAndFit(sizer);

    if (sizer->GetMinSize().y > 600)
    {
        this->SetMinSize(wxSize(sizer->GetMinSize().x, 600));
    }
}

map<string, map<Slot, set<Slot>>> BaseSelection::getBaseSlots()
{
    map<string, map<Slot, set<Slot>>> slots;
    auto addSlots = mHandler->getAddSlots();

    int index = 0;
    for (auto i = addSlots.begin(); i != addSlots.end(); i++)
    {
        for (auto j = i->second.begin(); j != i->second.end(); j++)
        {
            slots[i->first][Slot(baseSlots[index]->GetStringSelection().ToStdString())].insert(*j);
            index++;
        }
    }

    return slots;
}
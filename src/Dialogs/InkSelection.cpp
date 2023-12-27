#include "InkSelection.h"

InkSelection::InkSelection
(
    wxWindow* parent, wxWindowID id,
    const wxString& title,
    ModHandler* mHandler,
    Settings settings,
    const wxPoint& pos,
    const wxSize& size,
    long style,
    const wxString& name
) : wxDialog(parent, id, title, pos, size, style, name)
{
    this->mHandler = mHandler;

    auto allSlots = mHandler->getAllSlots(false);
    auto inkSlots = allSlots.find("inkling");

    list.Add("c00");
    list.Add("c01");
    list.Add("c02");
    list.Add("c03");
    list.Add("c04");
    list.Add("c05");
    list.Add("c06");
    list.Add("c07");

    if (inkSlots != allSlots.end())
    {
        for (auto i = inkSlots->second.begin(); i != inkSlots->second.end(); i++)
        {
            list.Add("c" + i->getString());
        }
    }

    map<Slot, InklingColor> inklingColors;

    if (settings.readInk)
    {
        inklingColors = mHandler->read_ink_colors();
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
    sizerM->Add(sizerV, 0, wxALIGN_CENTER_HORIZONTAL | wxLEFT | wxTOP | wxRIGHT, FromDIP(20));

    int index = 0;

    if (settings.readInk)
    {
        for (auto i = inklingColors.begin(); i != inklingColors.end(); i++)
        {
            // Create new section after every 4 boxes
            if (index % 3 == 0)
            {
                sizerH = new wxBoxSizer(wxHORIZONTAL);
                sizerM->Add(sizerH, 1, wxALIGN_CENTER_HORIZONTAL | wxLEFT | wxTOP | wxRIGHT, FromDIP(20));
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

            wxChoice* slotList = new wxChoice(this, wxID_ANY, wxDefaultPosition, FromDIP(wxSize(50, -1)), list);
            sizerV->Add(slotList, 1, wxALIGN_CENTER_HORIZONTAL);
            finalSlots.push_back(slotList);

            slotList->SetStringSelection("c" + i->first.getString());

            index++;
        }
    }

    // Add Button
    add.sizer = new wxBoxSizer(wxVERTICAL);
    add.button = new wxButton(this, wxID_ANY, "+", wxDefaultPosition, FromDIP(wxSize(40, -1)));

    add.sizer->Add(add.button, 1, wxALIGN_CENTER_HORIZONTAL);
    add.button->Bind(wxEVT_BUTTON, &InkSelection::onAddPressed, this);

    add.list = new wxChoice(this, wxID_ANY, wxDefaultPosition, FromDIP(wxSize(50, -1)), list);
    add.list->Select(0);
    add.sizer->Add(add.list, 1, wxALIGN_CENTER_HORIZONTAL);

    if (sizerH == nullptr || sizerH->GetItemCount() == 3)
    {
        sizerH = new wxBoxSizer(wxHORIZONTAL);
        sizerH->Add(add.sizer, 1, wxALIGN_CENTER_VERTICAL);

        sizerM->Add(sizerH, 1, wxALIGN_CENTER_HORIZONTAL | wxLEFT | wxTOP | wxRIGHT, FromDIP(20));
    }
    else
    {
        sizerH->Add(add.sizer, 1, wxALIGN_CENTER_VERTICAL | wxLEFT, FromDIP(20));
    }

    sizerM->Add(CreateButtonSizer(wxOK | wxCANCEL), 0, wxALIGN_RIGHT | wxALL, FromDIP(20));
    this->SetSizerAndFit(sizerM);

    this->SetClientSize(FromDIP(wxSize(350, -1)));
    this->SetMinSize(FromDIP(wxSize(350, -1)));
}


void InkSelection::onAddPressed(wxCommandEvent& evt)
{
    InklingColor color;
    color = mHandler->getInklingColor(Slot(add.list->GetStringSelection().ToStdString()));

    sizerV = new wxBoxSizer(wxVERTICAL);

    wxColourPickerCtrl* effectPicker = new wxColourPickerCtrl(this, wxID_ANY, wxStockGDI::COLOUR_BLUE, wxDefaultPosition, wxDefaultSize);
    effectPicker->SetColour(color.effect);
    sizerV->Add(effectPicker, 1, wxALIGN_CENTER_HORIZONTAL);
    finalEffects.push_back(effectPicker);

    wxColourPickerCtrl* arrowPicker = new wxColourPickerCtrl(this, wxID_ANY, wxStockGDI::COLOUR_BLUE, wxDefaultPosition, wxDefaultSize);
    arrowPicker->SetColour(color.arrow);
    sizerV->Add(arrowPicker, 1, wxALIGN_CENTER_HORIZONTAL);
    finalArrows.push_back(arrowPicker);

    wxChoice* slotList = new wxChoice(this, wxID_ANY, wxDefaultPosition, FromDIP(wxSize(50, -1)), list);
    slotList->SetSelection(add.list->GetSelection());
    sizerV->Add(slotList, 1, wxALIGN_CENTER_HORIZONTAL);
    finalSlots.push_back(slotList);

    sizerH->Insert(sizerH->GetItemCount() - 1, sizerV, 1, wxALIGN_CENTER_VERTICAL);

    if (sizerH->GetItemCount() == 4)
    {
        sizerH->Detach(add.sizer);

        sizerH = new wxBoxSizer(wxHORIZONTAL);
        sizerH->Add(add.sizer, 1, wxALIGN_CENTER_VERTICAL);

        sizerM->Insert(sizerM->GetItemCount() - 1, sizerH, 1, wxALIGN_CENTER_HORIZONTAL | wxLEFT | wxTOP | wxRIGHT, FromDIP(20));
    }

    SendSizeEvent();

    // Adjust Size for best fit
    if (this->GetSize().x < this->GetBestSize().x)
    {
        this->SetSize(FromDIP(wxSize(this->GetBestSize().x, this->GetSize().y)));
    }
    if (this->GetSize().y < this->GetBestSize().y)
    {
        this->SetSize(FromDIP(wxSize(this->GetSize().x, this->GetBestSize().y)));
    }
}

map<Slot, InklingColor> InkSelection::getFinalColors()
{
    map<Slot, InklingColor> finalColors;
    for (int i = 0; i < finalSlots.size(); i++)
    {
        finalColors[Slot(finalSlots[i]->GetStringSelection().ToStdString())].effect = finalEffects[i]->GetColour();
        finalColors[Slot(finalSlots[i]->GetStringSelection().ToStdString())].arrow = finalArrows[i]->GetColour();
    }
    return finalColors;
}

#include "RenameSelection.h"

RenameSelection::RenameSelection(wxWindow* parent, wxWindowID id,
    const wxString& title,
    ModHandler* mHandler,
    const wxPoint& pos,
    const wxSize& size,
    long style,
    const wxString& name) :
    wxDialog(parent, id, title, pos, size, style, name)
{
    this->mHandler = mHandler;
    auto chars = mHandler->wxGetCharacterNames();

    wxBoxSizer* sizerM = new wxBoxSizer(wxVERTICAL);
    wxBoxSizer* sizerH = nullptr;

    for (const auto& name : chars)
    {
        names.push_back(new wxTextCtrl(this, wxID_ANY, mHandler->getCode(name.ToStdString())));

        sizerH = new wxBoxSizer(wxHORIZONTAL);
        sizerH->Add(new wxStaticText(this, wxID_ANY, name + ":"), 1, wxALIGN_CENTER_VERTICAL);
        sizerH->Add(names[names.size() - 1], 1, wxALIGN_CENTER_VERTICAL | wxLEFT, 20);

        sizerM->Add(sizerH, 1, wxALIGN_CENTER_HORIZONTAL | wxLEFT | wxTOP | wxRIGHT, 20);
    }

    sizerM->Add(CreateButtonSizer(wxOK | wxCANCEL), 0, wxALIGN_CENTER_VERTICAL | wxALL, FromDIP(20));
    this->SetSizerAndFit(sizerM);
}

map<string, string> RenameSelection::getNames()
{
    map<string, string> result;
    auto chars = mHandler->wxGetCharacterNames();

    int index = 0;
    for (const auto& name : chars)
    {
        if (mHandler->getCode(name.ToStdString()) != names[index]->GetValue())
        {
            result[mHandler->getCode(name.ToStdString())] = names[index]->GetValue();
        }
    }

    return result;
}
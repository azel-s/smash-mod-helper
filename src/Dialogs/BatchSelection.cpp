#include "BatchSelection.h"
#include "BaseSelection.h"
#include "PrcSelection.h"
#include <filesystem>
namespace fs = std::filesystem;

BatchSelection::BatchSelection
(
	wxWindow* parent,
	wxWindowID id,
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
	this->settings = settings;

	wxBoxSizer* sizerM = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer* sizerH = new wxBoxSizer(wxHORIZONTAL);

	string folderName = fs::path(mHandler->getPath()).filename().string();
	wxStaticText* text = new wxStaticText(this, wxID_ANY, "Mod Folder: " + folderName);

	bool hasAddSlot = mHandler->hasAddSlot();

	if (hasAddSlot || settings.baseSource)
	{
		base = new wxButton(this, wxID_ANY, "Select Base Slots", wxDefaultPosition, wxDefaultSize);
		base->Bind(wxEVT_BUTTON, &BatchSelection::onBasePressed, this);
		base->SetToolTip("Choose source slot(s) for each additional slot");
		sizerH->Add(base, 1);
	}

	config = new wxButton(this, wxID_ANY, "Create Config", wxDefaultPosition, wxDefaultSize);
	config->Bind(wxEVT_BUTTON, &BatchSelection::onConfigPressed, this);
	config->SetToolTip("Create a config for any additionals costumes, extra textures, and/or effects");
	sizerH->Add(config, 1);

	prc = new wxButton(this, wxID_ANY, "Create PRCXML", wxDefaultPosition, wxDefaultSize);
	prc->Bind(wxEVT_BUTTON, &BatchSelection::onPrcPressed, this);
	prc->SetToolTip("Edit slot names or modify max slots for each character");
	sizerH->Add(prc, 1);

	wxButton* finish = new wxButton(this, wxID_ANY, "Finish", wxDefaultPosition, wxDefaultSize);
	finish->Bind(wxEVT_BUTTON, &BatchSelection::onFinishPressed, this);
	finish->SetToolTip("Finish this mod folder, and continue to the next.");

	sizerM->Add(text, 0, wxALL, FromDIP(20));
	sizerM->Add(sizerH, 0, wxALIGN_CENTER_HORIZONTAL | wxLEFT | wxTOP | wxRIGHT, FromDIP(20));
	sizerM->Add(finish, 0, wxEXPAND | wxALL, FromDIP(20));

	if (hasAddSlot || settings.baseSource)
	{
		base->Hide();
		this->SetSizerAndFit(sizerM);

		base->Show();
		config->Hide();
		prc->Hide();
	}
	else
	{
		this->SetSizerAndFit(sizerM);
	}
}

void BatchSelection::onBasePressed(wxCommandEvent& evt)
{
	BaseSelection dlg(this, wxID_ANY, "Choose Base Slots", mHandler, settings);

	if (dlg.ShowModal() == wxID_OK)
	{
		mHandler->setBaseSlots(dlg.getBaseSlots());
		base->Hide();
		config->Show();
		prc->Show();
	}

	this->SendSizeEvent();
}

void BatchSelection::onConfigPressed(wxCommandEvent& evt)
{
	mHandler->create_config();
}

void BatchSelection::onPrcPressed(wxCommandEvent& evt)
{
	PrcSelection dlg(this, wxID_ANY, "Make Selection", mHandler, settings);
	if (dlg.ShowModal() == wxID_OK)
	{
		auto maxSlots = dlg.getMaxSlots();
		auto cIndex = dlg.getDB("cIndex");
		auto nIndex = dlg.getDB("nIndex");
		auto cGroup = dlg.getDB("cGroup");
		auto finalAnnouncers = dlg.getAnnouncers();

		if (!maxSlots.empty() || !cIndex.empty() || !nIndex.empty() || !cGroup.empty() || !finalAnnouncers.empty())
		{
			mHandler->create_db_prcxml(cGroup, cIndex, nIndex, maxSlots, finalAnnouncers);
		}
		else if (fs::exists(mHandler->getPath() + "/ui/param/database/ui_chara_db.prcxml"))
		{
			fs::remove(mHandler->getPath() + "/ui/param/database/ui_chara_db.prcxml");
			mHandler->wxLog("> WARN: ui_chara_db.prcxml is not needed, previous one was deleted to avoid issues.");
		}
		else
		{
			mHandler->wxLog("> NOTE: ui_chara_db.prcxml is not needed.");
		}

		auto finalNames = dlg.getNames();
		if (!finalNames.empty())
		{
			mHandler->create_message_xmsbt(finalNames);
		}
		else if (fs::exists(mHandler->getPath() + "/ui/message/msg_name.xmsbt"))
		{
			fs::remove(mHandler->getPath() + "/ui/message/msg_name.xmsbt");
			mHandler->wxLog("> WARN: msg_name.xmsbt is not needed, previous one was deleted to avoid issues.");
		}
	}
}

void BatchSelection::onFinishPressed(wxCommandEvent& evt)
{
	this->Close();
}

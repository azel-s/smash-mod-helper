#include "MainFrame.h"
#include "Dialogs/BaseSelection.h"
#include "Dialogs/PrcSelection.h"
#include "Dialogs/InkSelection.h"
#include <filesystem>
#include <fstream>
#include <wx/wx.h>
namespace fs = std::filesystem;
using std::ofstream, std::string;

wxIMPLEMENT_APP(App);

bool App::OnInit()
{
	MainFrame* mainFrame = new MainFrame("Smash Ultimate Mod Helper");
	mainFrame->SetIcons(wxICON(SMASH_ICON));
	mainFrame->SetClientSize(mainFrame->FromDIP(wxSize(mainFrame->GetBestSize().x, mainFrame->GetBestSize().y * 14.0 / 24)));
	mainFrame->SetMinSize(mainFrame->FromDIP(mainFrame->GetSize()));
	mainFrame->Show();

	return true;
}

MainFrame::MainFrame(const wxString& title) :
	wxFrame
	(
		nullptr,
		wxID_ANY,
		title,
		wxDefaultPosition,
		wxDefaultSize,
		wxDEFAULT_FRAME_STYLE & ~wxMAXIMIZE_BOX
	),
	panel(new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize)),
	logWindow(new wxTextCtrl(panel, wxID_ANY, "Log Window:\n", wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE)),
	log(new wxLogTextCtrl(logWindow)),
	mHandler(log)
{
	/* --- Initial path --- */
	iPath = fs::current_path().string();
	replace(iPath.begin(), iPath.end(), '\\', '/');

	/* --- Log window --- */
	logWindow->SetEditable(false);
	logWindow->Show(false);

	/* --- Settings --- */
	readSettings();
	updateSettings();

	/* --- Menu bar--- */
	wxMenuBar* menuBar = new wxMenuBar();

	// File menu
	wxMenu* fileMenu = new wxMenu();
	this->Bind(wxEVT_MENU, &MainFrame::onBrowse, this, fileMenu->Append(wxID_ANY, "&Open Folder\tCtrl-O")->GetId(), wxID_ANY, new wxArgument("button"));
	this->Bind(wxEVT_MENU, &MainFrame::onMenuClose, this, fileMenu->Append(wxID_ANY, "Close\tAlt-F4")->GetId());

	// Tools menu
	wxMenu* toolsMenu = new wxMenu();
	inkMenu = new wxMenuItem(fileMenu, wxID_ANY, "Edit Inkling Colors", "Open a directory to enable this feature.");
	deskMenu = new wxMenuItem(fileMenu, wxID_ANY, "Delete Desktop.ini Files", "Open a directory to enable this feature.");
	this->Bind(wxEVT_MENU, &MainFrame::onInkPressed, this, toolsMenu->Append(inkMenu)->GetId());
	this->Bind(wxEVT_MENU, &MainFrame::onDeskPressed, this, toolsMenu->Append(deskMenu)->GetId());
	this->Bind(wxEVT_MENU, &MainFrame::onBatchPressed, this, toolsMenu->Append(wxID_ANY, "Batch Config/PRCXML")->GetId());
	inkMenu->Enable(false);
	deskMenu->Enable(false);

	// Options Menu
	wxMenu* optionsMenu = new wxMenu();

	wxMenu* loadFromMod = new wxMenu();
	optionsMenu->AppendSubMenu(loadFromMod, "Load from mod");
	auto readBaseID = loadFromMod->AppendCheckItem(wxID_ANY, "Base Slots", "Enables reading information from config.json")->GetId();
	auto readNameID = loadFromMod->AppendCheckItem(wxID_ANY, "Custom Names", "Enables reading information from msg_name.xmsbt")->GetId();
	auto readInkID = loadFromMod->AppendCheckItem(wxID_ANY, "Inkling Colors", "Enables reading information from effect.prcxml")->GetId();
	this->Bind(wxEVT_MENU, &MainFrame::toggleSetting, this, readBaseID, wxID_ANY, new wxArgument("base"));
	this->Bind(wxEVT_MENU, &MainFrame::toggleSetting, this, readNameID, wxID_ANY, new wxArgument("name"));
	this->Bind(wxEVT_MENU, &MainFrame::toggleSetting, this, readInkID, wxID_ANY, new wxArgument("ink"));
	loadFromMod->Check(readBaseID, settings.readBase);
	loadFromMod->Check(readNameID, settings.readNames);
	loadFromMod->Check(readInkID, settings.readInk);

	wxMenu* selectionType = new wxMenu();
	optionsMenu->AppendSubMenu(selectionType, "Selection type");
	auto selectUnionID = selectionType->AppendRadioItem(wxID_ANY, "Union", "Mario [c00 & c02] + Luigi [c00 + c03]-> [c01, c02, c03]")->GetId();
	auto selectIntersectID = selectionType->AppendRadioItem(wxID_ANY, "Intersect", "Mario [c00 & c02] + Luigi [c00 + c03] -> [c00]")->GetId();
	this->Bind(wxEVT_MENU, &MainFrame::toggleSetting, this, selectUnionID, wxID_ANY, new wxArgument("select"));
	this->Bind(wxEVT_MENU, &MainFrame::toggleSetting, this, selectIntersectID, wxID_ANY, new wxArgument("select"));
	selectionType->Check(selectUnionID, !settings.selectionType);
	selectionType->Check(selectIntersectID, settings.selectionType);

	menuBar->Append(fileMenu, "&File");
	menuBar->Append(toolsMenu, "&Tools");
	menuBar->Append(optionsMenu, "&Options");

	SetMenuBar(menuBar);

	/* --- Browse --- */
	browse.text = new wxTextCtrl(panel, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER);
	browse.text->Bind(wxEVT_TEXT_ENTER, &MainFrame::onBrowse, this, wxID_ANY, wxID_ANY, new wxArgument("text"));
	browse.button = new wxButton(panel, wxID_ANY, "Browse...", wxDefaultPosition, wxDefaultSize);
	browse.button->SetToolTip("Open folder containing fighter/ui/effect/sound folder(s)");
	browse.button->Bind(wxEVT_BUTTON, &MainFrame::onBrowse, this, wxID_ANY, wxID_ANY, new wxArgument("button"));

	/* --- Characters list --- */
	charsList = new wxListBox(panel, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxArrayString(), wxLB_MULTIPLE);
	charsList->Bind(wxEVT_LISTBOX, &MainFrame::onSelect, this, wxID_ANY, wxID_ANY, new wxArgument("char"));

	/* --- File type boxes --- */
	auto fTypes = mHandler.wxGetFileTypes();
	for (int i = 0; i < fTypes.size(); i++)
	{
		fileTypeBoxes.push_back(new wxCheckBox(panel, wxID_ANY, fTypes[i], wxDefaultPosition, wxDefaultSize));
		fileTypeBoxes[i]->Bind(wxEVT_CHECKBOX, &MainFrame::onSelect, this, wxID_ANY, wxID_ANY, new wxArgument("fileType"));
		fileTypeBoxes[i]->Disable();
	}

	/* --- Initial/Final list --- */
	initSlots.text = new wxStaticText(panel, wxID_ANY, "Initial Slot: ", wxDefaultPosition, FromDIP(wxSize(55, -1)));
	initSlots.list = new wxChoice(panel, wxID_ANY, wxDefaultPosition, wxSize(FromDIP(50), -1));
	initSlots.list->SetToolTip("Final Slot's source slot");

	finalSlots.text = new wxStaticText(panel, wxID_ANY, "Final Slot: ", wxDefaultPosition, FromDIP(wxSize(55, -1)));
	finalSlots.list = new wxSpinCtrl(panel, wxID_ANY, "", wxDefaultPosition, FromDIP(wxSize(50, -1)), wxSP_WRAP, 0, 255, 0);
	finalSlots.list->Bind(wxEVT_SPINCTRL, &MainFrame::onSelect, this, wxID_ANY, wxID_ANY, new wxArgument("finalSlot"));
	finalSlots.list->SetToolTip("Initial Slot's target slot");

	/* --- Buttons --- */
	buttons.mov = new wxButton(panel, wxID_ANY, "Move", wxDefaultPosition, wxDefaultSize);
	buttons.mov->Bind(wxEVT_BUTTON, &MainFrame::onActionPressed, this, wxID_ANY, wxID_ANY, new wxArgument("move"));
	buttons.mov->SetToolTip("Initial Slot is moved to Final Slot");
	buttons.mov->Disable();

	buttons.dup = new wxButton(panel, wxID_ANY, "Duplicate", wxDefaultPosition, wxDefaultSize);
	buttons.dup->Bind(wxEVT_BUTTON, &MainFrame::onActionPressed, this, wxID_ANY, wxID_ANY, new wxArgument("duplicate"));
	buttons.dup->SetToolTip("Initial Slot is duplicated to Final Slot");
	buttons.dup->Disable();

	buttons.del = new wxButton(panel, wxID_ANY, "Delete", wxDefaultPosition, wxDefaultSize);
	buttons.del->Bind(wxEVT_BUTTON, &MainFrame::onActionPressed, this, wxID_ANY, wxID_ANY, new wxArgument("delete"));
	buttons.del->SetToolTip("Initial Slot is deleted");
	buttons.del->Disable();

	buttons.log = new wxButton(panel, wxID_ANY, "Show Log", wxDefaultPosition, wxDefaultSize);
	buttons.log->Bind(wxEVT_BUTTON, &MainFrame::onLogPressed, this);
	buttons.log->SetToolTip("Log Window can help debug issues.");

	buttons.base = new wxButton(panel, wxID_ANY, "Select Base Slots", wxDefaultPosition, wxDefaultSize);
	buttons.base->Bind(wxEVT_BUTTON, &MainFrame::onBasePressed, this);
	buttons.base->SetToolTip("Choose source slot(s) for each additional slot");
	buttons.base->Hide();

	buttons.config = new wxButton(panel, wxID_ANY, "Create Config", wxDefaultPosition, wxDefaultSize);
	buttons.config->Bind(wxEVT_BUTTON, &MainFrame::onConfigPressed, this);
	buttons.config->SetToolTip("Create a config for any additionals costumes, extra textures, and/or effects");
	buttons.config->Hide();

	buttons.prcxml = new wxButton(panel, wxID_ANY, "Create PRCXML", wxDefaultPosition, wxDefaultSize);
	buttons.prcxml->Bind(wxEVT_BUTTON, &MainFrame::onPrcPressed, this);
	buttons.prcxml->SetToolTip("Edit slot names or modify max slots for each character");
	buttons.prcxml->Hide();

	/* --- Misc. --- */
	CreateStatusBar();

	/* --- Sizer setup --- */
	wxBoxSizer* sizerM = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer* sizerA = new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer* sizerB = new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer* sizerB1 = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer* sizerB2 = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer* sizerB3 = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer* sizerB3A = new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer* sizerB3B = new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer* sizerC = new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer* sizerC1 = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer* sizerC2 = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer* sizerC3 = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer* sizerD = new wxBoxSizer(wxHORIZONTAL);

	// Main Sizer
	sizerM->Add(sizerA, 1, wxEXPAND | wxALL, FromDIP(20));
	sizerM->Add(sizerB, 10, wxALIGN_CENTER_HORIZONTAL | wxLEFT | wxBOTTOM | wxRIGHT, FromDIP(20));
	sizerM->Add(sizerC, 1, wxEXPAND | wxLEFT | wxBOTTOM | wxRIGHT, FromDIP(20));
	sizerM->Add(sizerD, 10, wxEXPAND | wxLEFT | wxBOTTOM | wxRIGHT, FromDIP(20));

	// A
	sizerA->Add(browse.text, 1, wxRIGHT, FromDIP(10));
	sizerA->Add(browse.button, 0);

	// B1
	sizerB->Add(sizerB1, 3, wxEXPAND | wxRIGHT, FromDIP(20));
	sizerB1->Add(charsList, 1, wxEXPAND);

	// B2
	sizerB->Add(sizerB2, 1, wxALIGN_CENTER_VERTICAL | wxRIGHT, FromDIP(20));
	for (int i = 0; i < fileTypeBoxes.size() - 1; i++)
	{
		sizerB2->Add(fileTypeBoxes[i], 1);
		sizerB2->AddSpacer(FromDIP(20));
	}
	sizerB2->Add(fileTypeBoxes[fTypes.size() - 1], 1);

	// B3
	sizerB->Add(sizerB3, 1, wxALIGN_CENTER_VERTICAL);

	// B3A
	sizerB3->Add(sizerB3A, 1, wxBOTTOM, FromDIP(10));
	sizerB3A->Add(initSlots.text, 0, wxALIGN_CENTER_VERTICAL);
	sizerB3A->Add(initSlots.list, 1, wxALIGN_CENTER_VERTICAL);

	// B3B
	sizerB3->Add(sizerB3B, 1, wxBOTTOM, FromDIP(10));
	sizerB3B->Add(finalSlots.text, 0, wxALIGN_CENTER_VERTICAL);
	sizerB3B->Add(finalSlots.list, 1, wxALIGN_CENTER_VERTICAL);

	// B3(C)
	sizerB3->Add(buttons.mov, 1, wxEXPAND);
	sizerB3->Add(buttons.dup, 1, wxEXPAND);
	sizerB3->Add(buttons.del, 1, wxEXPAND);

	// C
	sizerC->Add(buttons.log, 1);
	sizerC->Add(buttons.base, 1);
	sizerC->Add(buttons.config, 1);
	sizerC->Add(buttons.prcxml, 1);

	// D
	sizerD->Add(logWindow, 1, wxEXPAND);

	panel->SetSizerAndFit(sizerM);
}

/* --- HELPER FUNCTIONS --- */
void MainFrame::updateControls(bool character, bool fileType, bool initSlot, bool finalSlot, bool newAddSlot, bool newInkSlot)
{
	wxArrayString codes;
	wxArrayString fileTypes;

	bool fileTypesFilled = false;
	bool codesFilled = false;

	if (character)
	{
		// Old names
		wxArrayInt selections;
		charsList->GetSelections(selections);
		set<wxString> selectionSet;
		for (auto& selection : selections)
		{
			selectionSet.insert(charsList->GetString(selection));
		}

		// New names
		auto newNames = mHandler.wxGetCharacterNames();
		charsList->Set(newNames);

		// Reselect old names
		for (int i = 0; i < newNames.size(); i++)
		{
			if (selectionSet.find(newNames[i]) != selectionSet.end())
			{
				charsList->Select(i);
			}
		}

		if (charsList->IsEmpty())
		{
			buttons.base->Hide();
			buttons.config->Hide();
			buttons.prcxml->Hide();
		}

		fileType = true;
	}

	if (fileType)
	{
		codes = getSelectedCharCodes();

		// Enable or disable file type checkbox based on whether or not it exists in character's map
		wxArrayString selectablefileTypes = mHandler.wxGetFileTypes(codes, settings.selectionType);
		auto allFileTypes = mHandler.wxGetFileTypes();
		for (int i = 0, j = 0; i < allFileTypes.size(); i++)
		{
			if (j < selectablefileTypes.size() && selectablefileTypes[j] == allFileTypes[i])
			{
				fileTypeBoxes[i]->Enable();
				j++;
			}
			else
			{
				fileTypeBoxes[i]->Disable();
				fileTypeBoxes[i]->SetValue(false);
			}
		}

		fileTypes = getSelectedFileTypes();
		fileTypesFilled = true;

		initSlot = true;
	}

	if (initSlot)
	{
		if (!codesFilled)
		{
			codes = getSelectedCharCodes();
		}
		if (!fileTypesFilled)
		{
			fileTypes = getSelectedFileTypes();
		}

		string oldSlot = (initSlots.list->GetSelection() != wxNOT_FOUND) ? initSlots.list->GetStringSelection().ToStdString() : "";

		initSlots.list->Set(mHandler.wxGetSlots(codes, fileTypes, settings.selectionType));
		if (!initSlots.list->IsEmpty())
		{
			if (oldSlot.empty())
			{
				oldSlot = initSlots.list->GetString(0);
			}

			if (!initSlots.list->SetStringSelection(oldSlot))
			{
				initSlots.list->SetSelection(0);
			}
		}
		else
		{
			buttons.mov->Disable();
			buttons.dup->Disable();
			buttons.del->Disable();
		}
	}

	if (finalSlot)
	{
		if (!codesFilled)
		{
			codes = getSelectedCharCodes();
		}
		if (!fileTypesFilled)
		{
			fileTypes = getSelectedFileTypes();
		}

		if (initSlots.list->GetSelection() != wxNOT_FOUND)
		{
			if (mHandler.wxHasSlot(codes, Slot(finalSlots.list->GetValue()), fileTypes, false))
			{
				buttons.mov->Disable();
				buttons.dup->Disable();
				buttons.del->Enable();
			}
			else
			{
				buttons.mov->Enable();
				buttons.dup->Enable();
				buttons.del->Enable();
			}
		}
		else
		{
			buttons.mov->Disable();
			buttons.dup->Disable();
			buttons.del->Disable();
		}
	}

	if (newAddSlot)
	{
		buttons.base->Show();
		buttons.config->Hide();
		buttons.prcxml->Hide();
	}

	if (!mHandler.getPath().empty())
	{
		if (newInkSlot)
		{
			inkMenu->Enable(false);
			inkMenu->SetHelp("Select base slots to enable this feature.");
		}
		else if(!mHandler.hasAddSlot("inkling"))
		{
			inkMenu->Enable();
			inkMenu->SetHelp("Add or modify colors. Required for additional slots.");
		}

		deskMenu->Enable();
		deskMenu->SetHelp("desktop.ini files can cause file-conflict issues.");
	}
	else
	{
		buttons.base->Hide();
		buttons.config->Hide();
		buttons.prcxml->Hide();

		inkMenu->Enable(false);
		inkMenu->SetHelp("Open a directory to enable this feature.");

		deskMenu->Enable(false);
		deskMenu->SetHelp("Open a directory to enable this feature.");
	}

	panel->SendSizeEvent();
}

void MainFrame::processPRCXML(ModHandler* mHandler)
{
	if (!mHandler)
	{
		mHandler = &(this->mHandler);
	}

	if (mHandler->hasChar())
	{
		PrcSelection dlg(this, wxID_ANY, "Make Selection", *mHandler, settings.readNames);
		if (dlg.ShowModal() == wxID_OK)
		{
			auto finalSlots = dlg.getMaxSlots(mHandler);
			auto finalNames = dlg.getNames();
			auto finalAnnouncers = dlg.getAnnouncers();

			wxArrayString exeLog;

			if (!finalSlots.empty() || !finalNames.empty() || !finalAnnouncers.empty())
			{
				// Change working directory
				wxSetWorkingDirectory("Files/prc/");

				// Create XML
				wxExecute("param-xml disasm vanilla.prc -o ui_chara_db.xml -l ParamLabels.csv", exeLog, exeLog, wxEXEC_SYNC | wxEXEC_NODISABLE);

				mHandler->create_db_prcxml(finalNames, finalAnnouncers, finalSlots);
				if (!finalNames.empty())
				{
					mHandler->create_message_xmsbt(finalNames);
				}

				if (exeLog.size() == 1 && exeLog[0].substr(0, 9) == "Completed")
				{
					log->LogText("> Success! ui_chara_db.prcxml was created!");

					if (!finalNames.empty())
					{
						log->LogText("> Success! msg_name.xmsbt was created!");
					}

					fs::create_directories(mHandler->getPath() + "/ui/param/database/");
					fs::rename(fs::current_path() / "ui_chara_db.prcxml", mHandler->getPath() + "/ui/param/database/ui_chara_db.prcxml");

					if (fs::exists(mHandler->getPath() + "/ui/param/database/ui_chara_db.prcx"))
					{
						fs::remove(mHandler->getPath() + "/ui/param/database/ui_chara_db.prcx");
					}
				}
				else
				{
					log->LogText("> Error: Something went wrong, possible issue below:");

					for (int i = 0; i < exeLog.size(); i++)
					{
						log->LogText(">" + exeLog[i]);
					}
				}

				// Restore working directory
				wxSetWorkingDirectory("../../");
			}
			else if (fs::exists(mHandler->getPath() + "/ui/param/database/ui_chara_db.prcxml"))
			{
				fs::remove(mHandler->getPath() + "/ui/param/database/ui_chara_db.prcxml");
				log->LogText("> NOTE: ui_chara_db.prcxml is not needed, previous one was deleted.");
			}
			else
			{
				log->LogText("> NOTE: ui_chara_db.prcxml is not needed.");
			}
		}
	}
	else
	{
		log->LogText("> N/A: Mod is empty, cannot create a prcx!");
	}
}

void MainFrame::readSettings()
{
	ifstream settingsFile(iPath + "/Files/settings.data");
	if (settingsFile.is_open())
	{
		int bit;

		settingsFile >> bit;
		settings.selectionType = (bit == 1) ? true : false;
		settingsFile >> bit;
		settings.readBase = (bit == 1) ? true : false;
		settingsFile >> bit;
		settings.readNames = (bit == 1) ? true : false;
		settingsFile >> bit;
		settings.readInk = (bit == 1) ? true : false;

		settingsFile.close();
	}
	else
	{
		log->LogText("Files/settings.data could not be loaded, using default values.");
	}
}

void MainFrame::updateSettings()
{
	ofstream settingsFile(iPath + "/Files/settings.data");
	if (settingsFile.is_open())
	{
		settingsFile << settings.selectionType << ' ';
		settingsFile << settings.readBase << ' ';
		settingsFile << settings.readNames << ' ';
		settingsFile << settings.readInk;

		settingsFile.close();
	}
	else
	{
		log->LogText("Files/settings.data could not be loaded, settings will not be saved.");
	}
}

/* --- BOUND FUNCTIONS --- */
void MainFrame::toggleSetting(wxCommandEvent& evt)
{
	string setting = static_cast<wxArgument*>(evt.GetEventUserData())->str;

	if (setting == "base")
	{
		settings.readBase = !settings.readBase;
		if (settings.readBase)
		{
			log->LogText("> Base slots will now be read from mods.");
		}
		else
		{
			log->LogText("> Base slots will NOT be read from mods.");
		}
	}
	else if (setting == "name")
	{
		settings.readNames = !settings.readNames;
		if (settings.readNames)
		{
			log->LogText("> Custom names will now be read from mods.");
		}
		else
		{
			log->LogText("> Custom names will NOT be read from mods.");
		}
	}
	else if (setting == "ink")
	{
		settings.readInk = !settings.readInk;
		if (settings.readInk)
		{
			log->LogText("> Inkling colors will now be read from mods.");
		}
		else
		{
			log->LogText("> Inkling colors will NOT be read from mods.");
		}
	}
	else if (setting == "select")
	{
		settings.selectionType = !settings.selectionType;
		if (settings.selectionType)
		{
			log->LogText("> Selection type is now set to intersect.");
		}
		else
		{
			log->LogText("> Selection type is now set to union.");
		}
		updateControls();
	}

	updateSettings();
}

void MainFrame::onBrowse(wxCommandEvent& evt)
{
	string path = "";

	if (static_cast<wxArgument*>(evt.GetEventUserData())->str == "text")
	{
		if (fs::is_directory(browse.text->GetValue().ToStdString()))
		{
			path = browse.text->GetValue();
		}
	}
	else
	{
		wxDirDialog dialog(this, "Choose the root directory of your mod...", "", wxDD_DEFAULT_STYLE | wxDD_DIR_MUST_EXIST);
		if (dialog.ShowModal() != wxID_CANCEL)
		{
			path = dialog.GetPath();
		}
	}

	if (!path.empty())
	{
		mHandler.readFiles(path);
		browse.text->SetValue(mHandler.getPath());
		updateControls(true, true, true, true, mHandler.hasAddSlot(), mHandler.hasAddSlot("inkling"));
	}
}

void MainFrame::onSelect(wxCommandEvent& evt)
{
	string select = static_cast<wxArgument*>(evt.GetEventUserData())->str;

	if (select == "char")
	{
		updateControls(false, true);
	}
	else if (select == "fileType")
	{
		updateControls(false, false, true);
	}
	else if (select == "finalSlot")
	{
		updateControls(false, false, false, true);
	}
}

void MainFrame::onActionPressed(wxCommandEvent& evt)
{
	wxArgument* arg = static_cast<wxArgument*>(evt.GetEventUserData());
	arg->num = mHandler.getNumCharacters();

	Slot initSlot = Slot(initSlots.list->GetStringSelection().ToStdString());
	Slot finalSlot = Slot(finalSlots.list->GetValue());

	auto fileTypes = getSelectedFileTypes();
	bool inkHasSlot = mHandler.wxHasSlot("inkling", initSlot, fileTypes);

	mHandler.adjustFiles(arg->str, getSelectedCharCodes(), fileTypes, initSlot, finalSlot);

	if (arg->str != "delete")
	{
		updateControls(false, false, true, true, finalSlots.list->GetValue() > 7, finalSlots.list->GetValue() > 7 && inkHasSlot);
	}
	else
	{
		updateControls(mHandler.getNumCharacters() != arg->num, true, true, true, false, false);
	}
}

void MainFrame::onBatchPressed(wxCommandEvent& evt)
{
	wxDirDialog dialog(this, "Choose the directory containing multiple mod folders...", "", wxDD_DEFAULT_STYLE | wxDD_DIR_MUST_EXIST);
	if (dialog.ShowModal() != wxID_CANCEL)
	{
		for (const auto& modFolder : fs::directory_iterator(dialog.GetPath().ToStdString()))
		{
			ModHandler mod(log, modFolder.path().string());
			if (mod.hasChar())
			{
				if (fs::exists(mod.getPath() + "/config.json"))
				{
					auto readSlots = mod.read_config_slots();

					map<string, map<Slot, set<Slot>>> slots;
					for (auto i = readSlots.begin(); i != readSlots.end(); i++)
					{
						for (auto j = i->second.begin(); j != i->second.end(); j++)
						{
							slots[i->first][j->second].insert(j->first);
						}
					}
					mod.setBaseSlots(slots);
				}

				mod.create_config();
				
				if (wxMessageBox("Create PRCXML for " + modFolder.path().filename().string() + "? (Recommended)", "Create PRCXML?", wxYES_NO) == wxYES)
				{
					processPRCXML(&mod);
				}
			}
		}
	}
}

void MainFrame::onLogPressed(wxCommandEvent& evt)
{
	if (logWindow->IsShown())
	{
		logWindow->Show(false);
		this->SetSize(FromDIP(wxSize(this->GetSize().x, this->GetSize().y * 14.0 / 24)));
		// this->SetSize(FromDIP(wxSize(this->GetSize().x, this->GetSize().y - 200)));
		buttons.log->SetLabel("Show Log");
	}
	else
	{
		logWindow->Show(true);
		this->SetSize(FromDIP(wxSize(this->GetSize().x, this->GetSize().y * 24.0 / 14)));
		//this->SetSize(FromDIP(wxSize(this->GetSize().x, this->GetSize().y + 200)));
		buttons.log->SetLabel("Hide Log");
	}

	panel->SendSizeEvent();
}

void MainFrame::onBasePressed(wxCommandEvent& evt)
{
	BaseSelection dlg(this, wxID_ANY, "Choose Base Slots", mHandler, settings.readBase);

	if (dlg.ShowModal() == wxID_OK)
	{
		mHandler.setBaseSlots(dlg.getBaseSlots());

		buttons.base->Hide();
		buttons.config->Show();
		buttons.prcxml->Show();

		inkMenu->Enable();
		inkMenu->SetHelp("Add or modify colors. Required for additional slots.");

		panel->SendSizeEvent();
	}
}

void MainFrame::onConfigPressed(wxCommandEvent& evt)
{
	mHandler.create_config();
}

void MainFrame::onPrcPressed(wxCommandEvent& evt)
{
	processPRCXML(&mHandler);
}

void MainFrame::onInkPressed(wxCommandEvent& evt)
{
	InkSelection dlg(this, wxID_ANY, "Choose Inkling Colors", mHandler, settings.readInk);

	if (dlg.ShowModal() == wxID_OK)
	{
		auto finalColors = dlg.getFinalColors();

		if (!finalColors.empty())
		{
			// Change directory to parcel and param-xml's location
			// TODO: Make function not rely on directory change
			wxSetWorkingDirectory("Files/prc/");

			mHandler.create_ink_prcxml(finalColors);

			fs::create_directories(mHandler.getPath() + "/fighter/common/param/");
			fs::rename(fs::current_path() / "effect_Edit.prcxml", mHandler.getPath() + "/fighter/common/param/effect.prcxml");

			if (fs::exists(mHandler.getPath() + "/fighter/common/param/effect.prcx"))
			{
				fs::remove(mHandler.getPath() + "/fighter/common/param/effect.prcx");
			}

			// Restore working directory
			wxSetWorkingDirectory("../../");
		}
		else
		{
			log->LogText("> N/A: No changes were made.");
		}
	}
}

void MainFrame::onDeskPressed(wxCommandEvent& evt)
{
	mHandler.remove_desktop_ini();
}

void MainFrame::onMenuClose(wxCommandEvent& evt)
{
	Close(true);
}

/* --- GETTERS --- */
wxArrayString MainFrame::getSelectedCharCodes()
{
	wxArrayInt selections;
	charsList->GetSelections(selections);

	wxArrayString codes;
	for (auto& selection : selections)
	{
		codes.Add(mHandler.getCode(charsList->GetString(selection).ToStdString()));
	}
	return codes;
}

wxArrayString MainFrame::getSelectedFileTypes()
{
	wxArrayString result;
	for (auto& box : fileTypeBoxes)
	{
		if (box->IsChecked())
		{
			result.Add(box->GetLabel());
		}
	}
	return result;
}
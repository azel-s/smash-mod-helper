#include "MainFrame.h"
#include "BaseSelection.h"
#include "PrcSelection.h"
#include "InkSelection.h"
#include <wx/wx.h>
#include <wx/spinctrl.h>
#include <wx/dirdlg.h>
#include <filesystem>
#include <fstream>
#include <codecvt>
namespace fs = std::filesystem;
using std::string;
using std::string; using std::ofstream;

MainFrame::MainFrame(const wxString& title) : wxFrame(nullptr, wxID_ANY, title, wxDefaultPosition, wxDefaultSize, wxSYSTEM_MENU | wxCAPTION | wxCLOSE_BOX | wxMINIMIZE_BOX & ~wxRESIZE_BORDER)
{
	// Read Settings
	ifstream settingsFile("settings.ini");
	// TODO:
	settingsFile.close();

	// Create main panel
	panel = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize);

	// Setup Menu
	menuBar = new wxMenuBar();

	wxMenu* fileMenu = new wxMenu();
	this->Bind(wxEVT_MENU, &MainFrame::onBrowse, this, fileMenu->Append(wxID_ANY, "&Open Folder\tCtrl-O")->GetId());
	this->Bind(wxEVT_MENU, &MainFrame::onMenuClose, this, fileMenu->Append(wxID_ANY, "Close\tAlt-F4")->GetId());

	wxMenu* toolsMenu = new wxMenu();
	inkMenu = new wxMenuItem(fileMenu, wxID_ANY, "Edit Inkling Colors", "Open a directory to enable this feature.");
	deskMenu = new wxMenuItem(fileMenu, wxID_ANY, "Delete Desktop.ini Files", "Open a directory to enable this feature.");
	this->Bind(wxEVT_MENU, &MainFrame::onInkPressed, this, toolsMenu->Append(inkMenu)->GetId());
	this->Bind(wxEVT_MENU, &MainFrame::onDeskPressed, this, toolsMenu->Append(deskMenu)->GetId());
	inkMenu->Enable(false);
	deskMenu->Enable(false);

	wxMenu* optionsMenu = new wxMenu();

	wxMenu* prcOutput = new wxMenu();
	optionsMenu->AppendSubMenu(prcOutput, "Set PRC output");
	this->Bind(wxEVT_MENU, &MainFrame::togglePRCOutput, this, prcOutput->AppendRadioItem(wxID_ANY, "PRCXML", "This configuration is readable, but sacrifices load times.")->GetId());
	this->Bind(wxEVT_MENU, &MainFrame::togglePRCOutput, this, prcOutput->AppendRadioItem(wxID_ANY, "PRCX", "This configuration is faster, but sacrifices readability.")->GetId());

	wxMenu* loadFromMod = new wxMenu();
	optionsMenu->AppendSubMenu(loadFromMod, "Load from mod");
	auto readBaseID = loadFromMod->AppendCheckItem(wxID_ANY, "Base Slots", "Enables reading information from config.json")->GetId();
	auto readNameID = loadFromMod->AppendCheckItem(wxID_ANY, "Custom Names", "Enables reading information from msg_name.xmsbt")->GetId();
	auto readInkID = loadFromMod->AppendCheckItem(wxID_ANY, "Inkling Colors", "Enables reading information from effect.prcxml")->GetId();
	this->Bind(wxEVT_MENU, &MainFrame::toggleBaseReading, this, readBaseID);
	this->Bind(wxEVT_MENU, &MainFrame::toggleNameReading, this, readNameID);
	this->Bind(wxEVT_MENU, &MainFrame::toggleInkReading, this, readInkID);
	loadFromMod->Check(readBaseID, true);
	loadFromMod->Check(readNameID, true);
	loadFromMod->Check(readInkID, true);

	menuBar->Append(fileMenu, "&File");
	menuBar->Append(toolsMenu, "&Tools");
	menuBar->Append(optionsMenu, "&Options");

	SetMenuBar(menuBar);

	// Create browse button and text field
	browse.text = new wxTextCtrl(panel, wxID_ANY, "", wxDefaultPosition, wxDefaultSize);
	browse.button = new wxButton(panel, wxID_ANY, "Browse...", wxDefaultPosition, wxDefaultSize);
	browse.button->SetToolTip("Open folder containing fighter/ui/effect/sound folder(s)");
	browse.button->Bind(wxEVT_BUTTON, &MainFrame::onBrowse, this);
	browse.button->Disable();

	// Create log window
	logWindow = new wxTextCtrl(panel, wxID_ANY, "Log Window:\n", wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE);
	logWindow->SetEditable(false);
	logWindow->Show(settings.showLogWindow);
	log = new wxLogTextCtrl(logWindow);
	data.log = log;

	// TODO: Look into having less polling for the thread
	// Setup actions for background thread
	const auto readVanillaFiles = [this]()
	{
		data.readVanillaFiles("names");
		if (!data.stopVanillaThread)
		{
			browse.button->Enable();
		}

		data.readVanillaFiles("effect");
		data.readVanillaFiles("fighter");
		data.readVanillaFiles("camera");
		data.readVanillaFiles("result");
		data.vanillaThreadActive = false;

		if (!data.stopVanillaThread)
		{
			buttons.config->Enable();
			buttons.prc->Enable();
		}
	};

	// Start background thread
	data.vanillaThread = thread{ readVanillaFiles };
	data.vanillaThread.detach();
	data.vanillaThreadActive = true;

	// Create characters List
	charsList = new wxListBox(panel, wxID_ANY, wxDefaultPosition, wxDefaultSize);
	charsList->Bind(wxEVT_LISTBOX, &MainFrame::onCharSelect, this);

	// Create file type checkboxes
	fileTypeBoxes = new wxCheckBox * [data.fileTypes.size()];
	for (int i = 0; i < data.fileTypes.size(); i++)
	{
		fileTypeBoxes[i] = new wxCheckBox(panel, wxID_ANY, data.fileTypes[i], wxDefaultPosition, wxDefaultSize);
		fileTypeBoxes[i]->Bind(wxEVT_CHECKBOX, &MainFrame::onFileTypeSelect, this);
		fileTypeBoxes[i]->Disable();
	}

	// Create mod slot list
	initSlots.text = new wxStaticText(panel, wxID_ANY, "Initial Slot: ", wxDefaultPosition, wxSize(55, -1));
	initSlots.list = new wxChoice(panel, wxID_ANY, wxDefaultPosition, wxSize(50, -1));
	initSlots.list->Bind(wxEVT_CHOICE, &MainFrame::onModSlotSelect, this);
	initSlots.list->SetToolTip("Final Slot's source slot");

	// Create user slot List
	finalSlots.text = new wxStaticText(panel, wxID_ANY, "Final Slot: ", wxDefaultPosition, wxSize(55, -1));
	finalSlots.list = new wxSpinCtrl(panel, wxID_ANY, "", wxDefaultPosition, wxSize(50, -1), wxSP_WRAP, 0, 255, 0);
	finalSlots.list->Bind(wxEVT_SPINCTRL, &MainFrame::onUserSlotSelect, this);
	finalSlots.list->SetToolTip("Initial Slot's target slot");

	// Create Move Button
	buttons.mov = new wxButton(panel, wxID_ANY, "Move", wxDefaultPosition, wxDefaultSize);
	buttons.mov->Bind(wxEVT_BUTTON, &MainFrame::onMovePressed, this);
	buttons.mov->Disable();
	buttons.mov->SetToolTip("Initial Slot is moved to Final Slot");

	// Create Duplicate Button
	buttons.dup = new wxButton(panel, wxID_ANY, "Duplicate", wxDefaultPosition, wxDefaultSize);
	buttons.dup->Bind(wxEVT_BUTTON, &MainFrame::onDuplicatePressed, this);
	buttons.dup->Disable();
	buttons.dup->SetToolTip("Initial Slot is duplicated to Final Slot");

	// Create Delete Button
	buttons.del = new wxButton(panel, wxID_ANY, "Delete", wxDefaultPosition, wxDefaultSize);
	buttons.del->Bind(wxEVT_BUTTON, &MainFrame::onDeletePressed, this);
	buttons.del->Disable();
	buttons.del->SetToolTip("Initial Slot is deleted");

	// Create Log Button
	buttons.log = new wxButton(panel, wxID_ANY, "Show Log", wxDefaultPosition, wxDefaultSize);
	buttons.log->Bind(wxEVT_BUTTON, &MainFrame::onLogPressed, this);
	buttons.log->SetToolTip("Log Window can help debug issues.");

	// Create Base Slots Button
	buttons.base = new wxButton(panel, wxID_ANY, "Select Base Slots", wxDefaultPosition, wxDefaultSize);
	buttons.base->Bind(wxEVT_BUTTON, &MainFrame::onBasePressed, this);
	buttons.base->Hide();
	buttons.base->SetToolTip("Choose source slot(s) for each additional slot");

	// Create Config Button
	buttons.config = new wxButton(panel, wxID_ANY, "Create Config", wxDefaultPosition, wxDefaultSize);
	buttons.config->Bind(wxEVT_BUTTON, &MainFrame::onConfigPressed, this);
	buttons.config->Disable();
	buttons.config->Hide();
	buttons.config->SetToolTip("Creata a config for any additionals costumes, extra textures, and/or effects");

	// Create prcx Buttons
	buttons.prc = new wxButton(panel, wxID_ANY, "Create PRCXML", wxDefaultPosition, wxDefaultSize);
	buttons.prc->Bind(wxEVT_BUTTON, &MainFrame::onPrcPressed, this);
	buttons.prc->Disable();
	buttons.prc->Hide();
	buttons.prc->SetToolTip("Edit slot names or modify max slots for each character");

	// Set Close Window Bind
	this->Bind(wxEVT_CLOSE_WINDOW, &MainFrame::onClose, this);

	// Create Statusbar
	statusBar = CreateStatusBar();

	// Setup Sizer
	wxBoxSizer* sizerM = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer* sizerA = new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer* sizerB = new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer* sizerB1 = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer* sizerB2 = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer* sizerB3 = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer* sizerB3A = new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer* sizerB3B = new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer* sizerC = new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer* sizerD = new wxBoxSizer(wxHORIZONTAL);

	// Main Sizer
	sizerM->Add(sizerA, 0, wxEXPAND | wxALL, 20);
	sizerM->Add(sizerB, 0, wxALIGN_CENTER_HORIZONTAL | wxLEFT | wxBOTTOM | wxRIGHT, 20);
	sizerM->Add(sizerC, 0, wxLEFT | wxBOTTOM | wxRIGHT, 20);
	sizerM->Add(sizerD, 0, wxEXPAND | wxLEFT | wxBOTTOM | wxRIGHT, 20);

	// A
	sizerA->Add(browse.text, 1, wxRIGHT, 10);
	sizerA->Add(browse.button, 0);

	// B1
	sizerB->Add(sizerB1, 3, wxEXPAND | wxRIGHT, 20);
	sizerB1->Add(charsList, 1, wxEXPAND);

	// B2
	sizerB->Add(sizerB2, 1, wxALIGN_CENTER_VERTICAL | wxRIGHT, 20);
	for (int i = 0; i < data.fileTypes.size() - 1; i++)
	{
		sizerB2->Add(fileTypeBoxes[i], 1);
		sizerB2->AddSpacer(20);
	}
	sizerB2->Add(fileTypeBoxes[data.fileTypes.size() - 1], 1);

	// B3
	sizerB->Add(sizerB3, 1, wxALIGN_CENTER_VERTICAL);

	// B3A
	sizerB3->Add(sizerB3A, 1, wxBOTTOM, 10);
	sizerB3A->Add(initSlots.text, 0, wxALIGN_CENTER_VERTICAL);
	sizerB3A->Add(initSlots.list, 1, wxALIGN_CENTER_VERTICAL);

	// B3B
	sizerB3->Add(sizerB3B, 1, wxBOTTOM, 10);
	sizerB3B->Add(finalSlots.text, 0, wxALIGN_CENTER_VERTICAL);
	sizerB3B->Add(finalSlots.list, 1, wxALIGN_CENTER_VERTICAL);

	// B3(C)
	sizerB3->Add(buttons.mov, 1, wxEXPAND);
	sizerB3->Add(buttons.dup, 1, wxEXPAND);
	sizerB3->Add(buttons.del, 1, wxEXPAND);

	// C
	sizerC->Add(buttons.log, 1, wxALIGN_CENTER_VERTICAL);

	sizerC->AddStretchSpacer();
	sizerC->AddStretchSpacer();

	sizerC->Add(buttons.config, 1, wxALIGN_CENTER_VERTICAL);
	sizerC->Add(buttons.prc, 1, wxALIGN_CENTER_VERTICAL);
	sizerC->Add(buttons.base, 1, wxALIGN_CENTER_VERTICAL);

	// D
	sizerD->Add(logWindow, 1, wxEXPAND);

	panel->SetSizerAndFit(sizerM);
}

void MainFrame::resetFileTypeBoxes()
{
	for (int i = 0; i < data.fileTypes.size(); i++)
	{
		fileTypeBoxes[i]->Disable();
		fileTypeBoxes[i]->SetValue(false);
	}
}

void MainFrame::resetButtons()
{
	buttons.mov->Disable();
	buttons.dup->Disable();
	buttons.del->Disable();
}

wxArrayString MainFrame::getSelectedFileTypes()
{
	wxArrayString result;

	for (int i = 0; i < data.fileTypes.size(); i++)
	{
		if (fileTypeBoxes[i]->IsChecked())
		{
			result.Add(data.fileTypes[i]);
		}
	}

	return result;
}

bool MainFrame::isFileTypeSelected()
{
	for (int i = 0; i < data.fileTypes.size(); i++)
	{
		if (fileTypeBoxes[i]->IsChecked())
		{
			return true;
		}
	}

	return false;
}

void MainFrame::updateFileTypeBoxes()
{
	if (charsList->GetSelection() != wxNOT_FOUND)
	{
		wxArrayString fileTypes = data.getFileTypes(data.charCodes[charsList->GetStringSelection().ToStdString()]);

		// Enable or disable file type checkbox based on whether or not it exists in character's map
		for (int i = 0, j = 0; i < data.fileTypes.size(); i++)
		{
			if (j < fileTypes.size() && fileTypes[j] == data.fileTypes[i])
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
	}
}

void MainFrame::updateButtons()
{
	if (initSlots.list->GetSelection() != wxNOT_FOUND)
	{
		try
		{
			string finalSlot = std::to_string(finalSlots.list->GetValue());
			wxArrayString fileTypes = this->getSelectedFileTypes();

			// Append 0 to final slot if it is one digit
			if (finalSlot.size() == 1)
			{
				finalSlot = '0' + finalSlot;
			}

			if (data.hasSlot(data.charCodes[charsList->GetStringSelection().ToStdString()], fileTypes, finalSlot))
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
		catch (...)
		{
			resetButtons();
		}
	}
	else
	{
		resetButtons();
	}

	panel->SendSizeEvent();
}

void MainFrame::updateInkMenu()
{
	// Update Inkling Menu
	if (data.mod.find("inkling") != data.mod.end() && data.hasAdditionalSlot("inkling"))
	{
		inkMenu->Enable(false);
		inkMenu->SetHelp("Select base slots to enable this feature.");
	}
	else
	{
		inkMenu->Enable();
	}
}

void MainFrame::togglePRCOutput(wxCommandEvent& evt)
{
	settings.prcxOutput = !settings.prcxOutput;

	if (settings.prcxOutput)
	{
		log->LogText("> Set PRC output to PRCX");
		buttons.prc->SetLabel("Create PRCX");
	}
	else
	{
		log->LogText("> Set PRC output to PRCXML");
		buttons.prc->SetLabel("Create PRCXML");
	}

	// TODO: Update Settings
}

void MainFrame::toggleBaseReading(wxCommandEvent& evt)
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

	// TODO: Update Settings
}

void MainFrame::toggleNameReading(wxCommandEvent& evt)
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

	// TODO: Update Settings
}

void MainFrame::toggleInkReading(wxCommandEvent& evt)
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

	// TODO: Update Settings
}

void MainFrame::onBrowse(wxCommandEvent& evt)
{
	wxDirDialog dialog(this, "Choose the root directory of your mod...", "", wxDD_DEFAULT_STYLE | wxDD_DIR_MUST_EXIST);

	if (dialog.ShowModal() != wxID_CANCEL)
	{
		// Reset previous information
		data.clear();
		charsList->Clear();
		initSlots.list->Clear();
		this->resetFileTypeBoxes();
		this->resetButtons();

		// Update path field
		browse.text->SetValue(dialog.GetPath());

		// Update data
		data.readData(dialog.GetPath().ToStdString());
		charsList->Set(data.getCharacters());

		// Update buttons
		if (data.hasAdditionalSlot())
		{
			buttons.base->Show();
			buttons.config->Hide();
			buttons.prc->Hide();
		}
		else
		{
			buttons.base->Hide();
			buttons.config->Show();
			buttons.prc->Show();
		}

		updateInkMenu();
		deskMenu->Enable();

		panel->SendSizeEvent();
	}
}

void MainFrame::onCharSelect(wxCommandEvent& evt)
{
	this->updateFileTypeBoxes();

	initSlots.list->Set(data.getSlots(data.charCodes[charsList->GetStringSelection().ToStdString()], getSelectedFileTypes()));

	if (!initSlots.list->IsEmpty())
	{
		initSlots.list->Select(0);
	}

	updateButtons();
}

void MainFrame::onFileTypeSelect(wxCommandEvent& evt)
{
	wxArrayString fileTypes = getSelectedFileTypes();

	initSlots.list->Set(data.getSlots(data.charCodes[charsList->GetStringSelection().ToStdString()], fileTypes));

	if (!initSlots.list->IsEmpty())
	{
		initSlots.list->Select(0);
	}

	updateButtons();
}

void MainFrame::onModSlotSelect(wxCommandEvent& evt)
{
	this->updateButtons();
}

void MainFrame::onUserSlotSelect(wxCommandEvent& evt)
{
	this->updateButtons();
}

void MainFrame::onMovePressed(wxCommandEvent& evt)
{
	wxArrayString fileTypes = getSelectedFileTypes();

	string initSlot = initSlots.list->GetStringSelection().ToStdString();
	string finalSlot = std::to_string(finalSlots.list->GetValue());

	// Append 0 to final slot if it is one digit
	if (finalSlot.size() == 1)
	{
		finalSlot = '0' + finalSlot;
	}

	data.adjustFiles("move", data.charCodes[charsList->GetStringSelection().ToStdString()], fileTypes, initSlot, finalSlot);
	initSlots.list->Set(data.getSlots(data.charCodes[charsList->GetStringSelection().ToStdString()], getSelectedFileTypes()));
	initSlots.list->SetStringSelection(finalSlot);

	this->updateButtons();

	// Update buttons
	if (data.hasAdditionalSlot())
	{
		buttons.base->Show();
		buttons.config->Hide();
		buttons.prc->Hide();
	}
	else
	{
		buttons.base->Hide();
		buttons.config->Show();
		buttons.prc->Show();
	}

	updateInkMenu();

	panel->SendSizeEvent();
}

void MainFrame::onDuplicatePressed(wxCommandEvent& evt)
{
	wxArrayString fileTypes = getSelectedFileTypes();

	string initSlot = initSlots.list->GetStringSelection().ToStdString();
	string finalSlot = std::to_string(finalSlots.list->GetValue());

	// Append 0 to final slot if it is one digit
	if (finalSlot.size() == 1)
	{
		finalSlot = '0' + finalSlot;
	}

	data.adjustFiles("duplicate", data.charCodes[charsList->GetStringSelection().ToStdString()], fileTypes, initSlot, finalSlot);
	initSlots.list->Set(data.getSlots(data.charCodes[charsList->GetStringSelection().ToStdString()], getSelectedFileTypes()));
	initSlots.list->SetStringSelection(initSlot);

	this->updateButtons();

	// Update buttons
	if (data.hasAdditionalSlot())
	{
		buttons.base->Show();
		buttons.config->Hide();
		buttons.prc->Hide();
	}

	updateInkMenu();

	panel->SendSizeEvent();
}

void MainFrame::onDeletePressed(wxCommandEvent& evt)
{
	int numChar = data.mod.size();

	data.adjustFiles("delete", data.charCodes[charsList->GetStringSelection().ToStdString()], getSelectedFileTypes(), initSlots.list->GetStringSelection().ToStdString(), "-1");

	if (data.mod.size() != numChar)
	{
		charsList->Set(data.getCharacters());
		this->resetFileTypeBoxes();
		initSlots.list->Set(wxArrayString());
		this->resetButtons();
	}
	else
	{
		this->updateFileTypeBoxes();
		initSlots.list->Set(data.getSlots(data.charCodes[charsList->GetStringSelection().ToStdString()], getSelectedFileTypes()));

		if (!initSlots.list->IsEmpty())
		{
			initSlots.list->SetSelection(0);
		}

		this->updateButtons();
	}

	// Update buttons
	if (!data.hasAdditionalSlot())
	{
		buttons.base->Hide();
		buttons.config->Show();
		buttons.prc->Show();
	}

	updateInkMenu();

	panel->SendSizeEvent();
}

void MainFrame::onLogPressed(wxCommandEvent& evt)
{
	settings.showLogWindow = !settings.showLogWindow;

	if (logWindow->IsShown())
	{
		logWindow->Show(false);
		this->SetSize(wxSize(this->GetSize().x, this->GetSize().y - 200));
		buttons.log->SetLabel("Show Log");
	}
	else
	{
		logWindow->Show(true);
		this->SetSize(wxSize(this->GetSize().x, this->GetSize().y + 200));
		buttons.log->SetLabel("Hide Log");
	}

	panel->SendSizeEvent();
}

void MainFrame::onBasePressed(wxCommandEvent& evt)
{
	BaseSelection dlg(this, wxID_ANY, "Choose Base Slots", data, settings.readBase);

	if (dlg.ShowModal() == wxID_OK)
	{
		data.baseSlots = dlg.getBaseSlots();

		// Update Buttons
		buttons.base->Hide();
		buttons.config->Show();
		buttons.prc->Show();

		inkMenu->Enable();

		panel->SendSizeEvent();
	}
}

void MainFrame::onConfigPressed(wxCommandEvent& evt)
{
	if (data.hasAdditionalSlot())
	{
		data.createConfig(data.baseSlots);
	}
	else
	{
		map<string, map<string, set<string>>> temp;
		data.createConfig(temp);
	}
}

void MainFrame::onInkPressed(wxCommandEvent& evt)
{
	auto inklingColors = data.readInk();

	InkSelection dlg(this, wxID_ANY, "Choose Inkling Colors", data, settings.readInk);

	if (dlg.ShowModal() == wxID_OK)
	{
		auto finalColors = dlg.getFinalColors();

		if (!finalColors.empty())
		{
			if (settings.prcxOutput)
			{
				wxArrayString exeLog;

				// Change directory to parcel and param-xml's location
				// INFO: parcel requires current working directory to be the same,
				wxSetWorkingDirectory("Files/prc/");

				// Create XML
				wxExecute("param-xml disasm effect.prc -o effect.xml -l ParamLabels.csv", exeLog, exeLog, wxEXEC_SYNC | wxEXEC_NODISABLE);

				// Patch Colors
				data.patchXMLInkColors(finalColors);

				// Create Modif PRC
				wxExecute("param-xml asm effect.xml -o modif_effect.prc -l ParamLabels.csv", exeLog, exeLog, wxEXEC_SYNC | wxEXEC_NODISABLE);

				// Create Modif PRCX
				wxExecute("parcel diff effect.prc modif_effect.prc effect.prcx", exeLog, exeLog, wxEXEC_SYNC | wxEXEC_NODISABLE);

				bool error = true;

				if (exeLog.size() == 2 && exeLog[0].substr(0, 9) == "Completed")
				{
					log->LogText("> Success! effect.prcx was created!");
					error = false;
				}

				if (error)
				{
					log->LogText("> Error: Something went wrong, possible issue below:");

					for (int i = 0; i < exeLog.size(); i++)
					{
						log->LogText(">" + exeLog[i]);
					}
				}
				else
				{
					fs::create_directories(data.rootPath + "/fighter/common/param/");
					fs::rename(fs::current_path() / "effect.prcx", data.rootPath + "/fighter/common/param/effect.prcx");

					fs::remove(fs::current_path() / "modif_effect.prc");

					if (fs::exists(data.rootPath + "/fighter/common/param/effect.prcxml"))
					{
						fs::remove(data.rootPath + "/fighter/common/param/effect.prcxml");
					}
					fs::remove(fs::current_path() / "effect.xml");
				}

				// Restore working directory
				wxSetWorkingDirectory("../../");
			}
			else
			{
				// Change directory to parcel and param-xml's location
				// TODO: Make function not rely on directory change
				wxSetWorkingDirectory("Files/prc/");

				data.createInkPRCXML(finalColors);

				fs::create_directories(data.rootPath + "/fighter/common/param/");
				fs::rename(fs::current_path() / "effect_Edit.prcxml", data.rootPath + "/fighter/common/param/effect.prcxml");

				if (fs::exists(data.rootPath + "/fighter/common/param/effect.prcx"))
				{
					fs::remove(data.rootPath + "/fighter/common/param/effect.prcx");
				}

				// Restore working directory
				wxSetWorkingDirectory("../../");
			}
		}
		else
		{
			log->LogText("> N/A: No changes were made.");
		}
	}
}

void MainFrame::onPrcPressed(wxCommandEvent& evt)
{
	bool hasAddSlots = data.hasAdditionalSlot();

	if (hasAddSlots || !data.mod.empty())
	{
		// Make character-slots map
		map<string, set<string>> allSlots;
		for (auto i = data.mod.begin(); i != data.mod.end(); i++)
		{
			wxArrayString slots = data.getSlots(i->first);

			for (auto j = slots.begin(); j != slots.end(); j++)
			{
				if (*j == "all")
				{
					continue;
				}

				allSlots[i->first].insert(j->ToStdString());
			}
		}

		PrcSelection dlg(this, wxID_ANY, "Make Selection", data, settings.readNames);
		if (dlg.ShowModal() == wxID_OK)
		{
			map<string, int> finalSlots = dlg.getMaxSlots(allSlots);
			map<string, map<int, Name>> finalNames = dlg.getNames();
			map<string, map<int, string>> finalAnnouncers = dlg.getAnnouncers();

			wxArrayString exeLog;

			if (hasAddSlots || !finalNames.empty() || !finalAnnouncers.empty())
			{
				if (settings.prcxOutput)
				{
					// Change directory to parcel and param-xml's location
					// INFO: parcel requires current working directory to be the same,
					wxSetWorkingDirectory("Files/prc/");

					// Create XML
					wxExecute("param-xml disasm vanilla.prc -o ui_chara_db.xml -l ParamLabels.csv", exeLog, exeLog, wxEXEC_SYNC | wxEXEC_NODISABLE);

					// Edit Vanilla XML
					if (hasAddSlots)
					{
						data.patchXMLSlots(finalSlots);
					}
					if (!finalNames.empty())
					{
						data.patchXMLNames(finalNames);
					}
					if (!finalAnnouncers.empty())
					{
						data.patchXMLAnnouncer(finalAnnouncers);
					}

					// Create Modif PRC
					wxExecute("param-xml asm ui_chara_db.xml -o modif.prc -l ParamLabels.csv", exeLog, exeLog, wxEXEC_SYNC | wxEXEC_NODISABLE);

					// Create Modif PRCX
					wxExecute("parcel diff vanilla.prc modif.prc ui_chara_db.prcx", exeLog, exeLog, wxEXEC_SYNC | wxEXEC_NODISABLE);

					if (exeLog.size() == 2 && exeLog[0].substr(0, 9) == "Completed")
					{
						log->LogText("> Success! ui_chara_db.prcx was created!");

						if (!finalNames.empty())
						{
							log->LogText("> Success! msg_name.xmsbt was created!");
						}

						fs::create_directories(data.rootPath + "/ui/param/database/");
						fs::rename(fs::current_path() / "ui_chara_db.prcx", data.rootPath + "/ui/param/database/ui_chara_db.prcx");

						fs::remove(fs::current_path() / "modif.prc");
						fs::remove(fs::current_path() / "ui_chara_db.xml");

						if (fs::exists(data.rootPath + "/ui/param/database/ui_chara_db.prcxml"))
						{
							fs::remove(data.rootPath + "/ui/param/database/ui_chara_db.prcxml");
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
				else
				{
					// Change directory to parcel and param-xml's location
					// INFO: parcel requires current working directory to be the same,
					wxSetWorkingDirectory("Files/prc/");

					// Create XML
					wxExecute("param-xml disasm vanilla.prc -o ui_chara_db.xml -l ParamLabels.csv", exeLog, exeLog, wxEXEC_SYNC | wxEXEC_NODISABLE);

					// Create PRCXML
					data.createPRCXML(finalNames, finalAnnouncers, finalSlots);

					if (exeLog.size() == 1 && exeLog[0].substr(0, 9) == "Completed")
					{
						log->LogText("> Success! ui_chara_db.prcx was created!");

						if (!finalNames.empty())
						{
							log->LogText("> Success! msg_name.xmsbt was created!");
						}

						fs::create_directories(data.rootPath + "/ui/param/database/");
						fs::rename(fs::current_path() / "ui_chara_db.prcxml", data.rootPath + "/ui/param/database/ui_chara_db.prcxml");


						if (fs::exists(data.rootPath + "/ui/param/database/ui_chara_db.prcx"))
						{
							fs::remove(data.rootPath + "/ui/param/database/ui_chara_db.prcx");
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
			}
			else
			{
				log->LogText("> N/A: No changes were made.");
			}
		}
	}
	else if (data.mod.empty())
	{
		log->LogText("> N/A: Mod is empty, cannot create a prcx!");
	}
	else
	{
		log->LogText("> N/A: No additional slots detected.");
	}
}

void MainFrame::onMenuClose(wxCommandEvent& evt)
{
	Close(true);
}

void MainFrame::onClose(wxCloseEvent& evt)
{
	// Stops background thread if it is still active
	if (data.vanillaThreadActive)
	{
		data.stopVanillaThread = true;

		while (data.vanillaThreadActive)
		{
			// Wait
		}
	}

	evt.Skip();
}

void MainFrame::onDeskPressed(wxCommandEvent& evt)
{
	data.removeDesktopINI();
}

MainFrame::~MainFrame()
{
	delete[] fileTypeBoxes;
}

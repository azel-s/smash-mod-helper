#include "MainFrame.h"
#include "Dialogs/BaseSelection.h"
#include "Dialogs/PrcSelection.h"
#include "Dialogs/InkSelection.h"
#include "Dialogs/CssRedirectSelection.h"
#include "Dialogs/BatchSelection.h"
#include <filesystem>
#include <fstream>
#include <wx/wx.h>
#include <wx/filename.h>
#include <wx/stdpaths.h>
#include <wx/gbsizer.h>
namespace fs = std::filesystem;
using std::ofstream, std::string;

wxIMPLEMENT_APP(App);

bool App::OnInit()
{
	wxImage::AddHandler(new wxPNGHandler);

	MainFrame* mainFrame = new MainFrame("Smash Ultimate Mod Helper", GetAppName().ToStdString());
	mainFrame->SetIcons(wxICON(SMASH_ICON));
	//mainFrame->SetSize(mainFrame->FromDIP(wxSize(mainFrame->GetBestSize().x, mainFrame->GetBestSize().y * 14.0 / 24)));
	//mainFrame->SetMinSize(mainFrame->FromDIP(mainFrame->GetSize()));
	mainFrame->Show();

	return true;
}

MainFrame::MainFrame(const wxString& title, string exe) :
	wxFrame
	(
		nullptr,
		wxID_ANY,
		title,
		wxDefaultPosition,
		wxDefaultSize,
		wxDEFAULT_FRAME_STYLE
	),
	splitter(new wxSplitterWindow(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxSP_BORDER | wxSP_LIVE_UPDATE)),
	lPanel(new wxPanel(splitter)),
	rPanel(new wxPanel(splitter)),
	logWindow(new wxTextCtrl(lPanel, wxID_ANY, "Log Window:\n", wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE)),
	log(new wxLogTextCtrl(logWindow)),
	mHandler(log)
{
	rPanel->SetBackgroundColour(wxColour(245, 245, 245));

	/* --- Initial path --- */
	this->exe = exe + ".exe";

	/* --- Log window --- */
	logWindow->SetEditable(false);

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
	inkMenu = new wxMenuItem(toolsMenu, wxID_ANY, "Edit Inkling Colors", "Open a directory to enable this feature.");
	cssMenu = new wxMenuItem(toolsMenu, wxID_ANY, "CSS Redirect", "Open a directory to enable this feature.");
	this->Bind(wxEVT_MENU, &MainFrame::onInkPressed, this, toolsMenu->Append(inkMenu)->GetId());
	this->Bind(wxEVT_MENU, &MainFrame::onCSSPressed, this, toolsMenu->Append(cssMenu)->GetId());
	this->Bind(wxEVT_MENU, &MainFrame::onBatchPressed, this, toolsMenu->Append(wxID_ANY, "Batch Config/PRCXML")->GetId());
	inkMenu->Enable(false);
	cssMenu->Enable(false);

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

	wxMenu* selection = new wxMenu();
	optionsMenu->AppendSubMenu(selection, "File type boxes");

	wxMenu* selectionType = new wxMenu();
	selection->AppendSubMenu(selectionType, "Selection type");
	auto selectUnionID = selectionType->AppendRadioItem(wxID_ANY, "Union", "Mario [c00 & c02] + Luigi [c00 + c03] -> [c01, c02, c03]")->GetId();
	auto selectIntersectID = selectionType->AppendRadioItem(wxID_ANY, "Intersect", "Mario [c00 & c02] + Luigi [c00 + c03] -> [c00]")->GetId();
	this->Bind(wxEVT_MENU, &MainFrame::toggleSetting, this, selectUnionID, wxID_ANY, new wxArgument("select"));
	this->Bind(wxEVT_MENU, &MainFrame::toggleSetting, this, selectIntersectID, wxID_ANY, new wxArgument("select"));
	selectionType->Check(selectUnionID, !settings.selectionType);
	selectionType->Check(selectIntersectID, settings.selectionType);

	auto selectAllByDefaultID = selection->AppendCheckItem(wxID_ANY, "Select all by default", "Enable to select all available file types by default on selection.")->GetId();
	this->Bind(wxEVT_MENU, &MainFrame::toggleSetting, this, selectAllByDefaultID, wxID_ANY, new wxArgument("selectAllByDefault"));
	selection->Check(selectAllByDefaultID, settings.selectAllByDefault);

	auto baseSouceID = optionsMenu->AppendCheckItem(wxID_ANY, "Base Slots for c00-c07", "Adjusts ui-info of base slots (e.g. Align Kazuya suit correctly on non-suit)")->GetId();
	this->Bind(wxEVT_MENU, &MainFrame::toggleSetting, this, baseSouceID, wxID_ANY, new wxArgument("baseSource"));
	optionsMenu->Check(baseSouceID, settings.baseSource);

	auto previewMenu = new wxMenuItem(optionsMenu, wxID_ANY, "UI Preview (Restart)", "Shows a preview panel to the right.", wxITEM_CHECK);
	this->Bind(wxEVT_MENU, &MainFrame::toggleSetting, this, optionsMenu->Append(previewMenu)->GetId(), wxID_ANY, new wxArgument("preview"));
	previewMenu->Check(settings.preview);

	menuBar->Append(fileMenu, "&File");
	menuBar->Append(toolsMenu, "&Tools");
	menuBar->Append(optionsMenu, "&Options");

	SetMenuBar(menuBar);

	/* --- Browse --- */
	browse.text = new wxTextCtrl(lPanel, wxID_ANY, "", wxDefaultPosition, wxSize(250, -1), wxTE_PROCESS_ENTER);
	browse.text->Bind(wxEVT_TEXT_ENTER, &MainFrame::onBrowse, this, wxID_ANY, wxID_ANY, new wxArgument("text"));
	browse.button = new wxButton(lPanel, wxID_ANY, "Browse...", wxDefaultPosition, wxDefaultSize);
	browse.button->SetToolTip("Open folder containing fighter/ui/effect/sound folder(s)");
	browse.button->Bind(wxEVT_BUTTON, &MainFrame::onBrowse, this, wxID_ANY, wxID_ANY, new wxArgument("button"));

	/* --- Characters list --- */
	charsList = new wxListBox(lPanel, wxID_ANY, wxDefaultPosition, wxSize(200, -1), wxArrayString(), wxLB_MULTIPLE | wxLB_SORT);
	charsList->Bind(wxEVT_LISTBOX, &MainFrame::onSelect, this, wxID_ANY, wxID_ANY, new wxArgument("char"));

	/* --- File type boxes --- */
	auto fTypes = mHandler.wxGetFileTypes();
	for (int i = 0; i < fTypes.size(); i++)
	{
		fileTypeBoxes.push_back(new wxCheckBox(lPanel, wxID_ANY, fTypes[i], wxDefaultPosition, wxDefaultSize));
		fileTypeBoxes[i]->Bind(wxEVT_CHECKBOX, &MainFrame::onSelect, this, wxID_ANY, wxID_ANY, new wxArgument("fileType"));
		fileTypeBoxes[i]->Disable();
	}

	/* --- Initial/Final list --- */
	initSlots.text = new wxStaticText(lPanel, wxID_ANY, "Initial Slot: ", wxDefaultPosition, FromDIP(wxSize(55, -1)));
	initSlots.list = new wxChoice(lPanel, wxID_ANY, wxDefaultPosition, FromDIP(wxSize(50, -1)));
	initSlots.list->Bind(wxEVT_CHOICE, &MainFrame::onSelect, this, wxID_ANY, wxID_ANY, new wxArgument("fileType"));
	initSlots.list->SetToolTip("Final Slot's source slot");

	finalSlots.text = new wxStaticText(lPanel, wxID_ANY, "Final Slot: ", wxDefaultPosition, FromDIP(wxSize(55, -1)));
	finalSlots.list = new wxSpinCtrl(lPanel, wxID_ANY, "", wxDefaultPosition, FromDIP(wxSize(50, -1)), wxSP_WRAP, 0, 255, 0);
	finalSlots.list->Bind(wxEVT_SPINCTRL, &MainFrame::onSelect, this, wxID_ANY, wxID_ANY, new wxArgument("finalSlot"));
	finalSlots.list->SetToolTip("Initial Slot's target slot");

	/* --- Buttons --- */
	buttons.mov = new wxButton(lPanel, wxID_ANY, "Move", wxDefaultPosition, wxDefaultSize);
	buttons.mov->Bind(wxEVT_BUTTON, &MainFrame::onActionPressed, this, wxID_ANY, wxID_ANY, new wxArgument("move"));
	buttons.mov->SetToolTip("Initial Slot is moved to Final Slot");
	buttons.mov->Disable();

	buttons.dup = new wxButton(lPanel, wxID_ANY, "Duplicate", wxDefaultPosition, wxDefaultSize);
	buttons.dup->Bind(wxEVT_BUTTON, &MainFrame::onActionPressed, this, wxID_ANY, wxID_ANY, new wxArgument("duplicate"));
	buttons.dup->SetToolTip("Initial Slot is duplicated to Final Slot");
	buttons.dup->Disable();

	buttons.del = new wxButton(lPanel, wxID_ANY, "Delete", wxDefaultPosition, wxDefaultSize);
	buttons.del->Bind(wxEVT_BUTTON, &MainFrame::onActionPressed, this, wxID_ANY, wxID_ANY, new wxArgument("delete"));
	buttons.del->SetToolTip("Initial Slot is deleted");
	buttons.del->Disable();

	buttons.log = new wxButton(lPanel, wxID_ANY, "Show Log", wxDefaultPosition, wxDefaultSize);
	buttons.log->Bind(wxEVT_BUTTON, &MainFrame::onLogPressed, this);
	buttons.log->SetToolTip("Log Window can help debug issues.");

	buttons.base = new wxButton(lPanel, wxID_ANY, "Select Base Slots", wxDefaultPosition, wxDefaultSize);
	buttons.base->Bind(wxEVT_BUTTON, &MainFrame::onBasePressed, this);
	buttons.base->SetToolTip("Choose source slot(s) for each additional slot");
	buttons.base->Hide();

	buttons.config = new wxButton(lPanel, wxID_ANY, "Create Config", wxDefaultPosition, wxDefaultSize);
	buttons.config->Bind(wxEVT_BUTTON, &MainFrame::onConfigPressed, this);
	buttons.config->SetToolTip("Create a config for any additionals costumes, extra textures, and/or effects");
	buttons.config->Hide();

	buttons.prcxml = new wxButton(lPanel, wxID_ANY, "Create PRCXML", wxDefaultPosition, wxDefaultSize);
	buttons.prcxml->Bind(wxEVT_BUTTON, &MainFrame::onPrcPressed, this);
	buttons.prcxml->SetToolTip("Edit slot names or modify max slots for each character");
	buttons.prcxml->Hide();

	/* --- Misc. --- */
	CreateStatusBar();

	/* --- Sizer setup --- */
	wxGridBagSizer* lBagSizer = new wxGridBagSizer(10, 10);
	wxGridBagSizer* rBagSizer = new wxGridBagSizer(0, 0);

	lBagSizer->Add(browse.text, wxGBPosition(0, 0), wxGBSpan(1, 5), wxEXPAND);
	lBagSizer->Add(browse.button, wxGBPosition(0, 5), wxGBSpan(1, 1), wxEXPAND);

	lBagSizer->Add(charsList, wxGBPosition(1, 0), wxGBSpan(6, 3), wxEXPAND);

	wxBoxSizer* sizerBoxes = new wxBoxSizer(wxVERTICAL);

	sizerBoxes->Add(fileTypeBoxes[0], 1, wxALIGN_LEFT);
	sizerBoxes->AddSpacer(10);
	sizerBoxes->Add(fileTypeBoxes[1], 1, wxALIGN_LEFT);
	sizerBoxes->AddSpacer(10);
	sizerBoxes->Add(fileTypeBoxes[2], 1, wxALIGN_LEFT);
	sizerBoxes->AddSpacer(10);
	sizerBoxes->Add(fileTypeBoxes[3], 1, wxALIGN_LEFT);

	lBagSizer->Add(sizerBoxes, wxGBPosition(1, 3), wxGBSpan(6, 1), wxALIGN_CENTER_VERTICAL);

	lBagSizer->Add(initSlots.text, wxGBPosition(1, 4), wxGBSpan(1, 1), wxALIGN_CENTER_VERTICAL);
	lBagSizer->Add(initSlots.list, wxGBPosition(1, 5), wxGBSpan(1, 1), wxEXPAND);

	lBagSizer->Add(finalSlots.text, wxGBPosition(2, 4), wxGBSpan(1, 1), wxALIGN_CENTER_VERTICAL);
	lBagSizer->Add(finalSlots.list, wxGBPosition(2, 5), wxGBSpan(1, 1), wxEXPAND);

	lBagSizer->Add(buttons.mov, wxGBPosition(4, 4), wxGBSpan(1, 2), wxEXPAND);
	lBagSizer->Add(buttons.dup, wxGBPosition(5, 4), wxGBSpan(1, 2), wxEXPAND);
	lBagSizer->Add(buttons.del, wxGBPosition(6, 4), wxGBSpan(1, 2), wxEXPAND);

	wxBoxSizer* sizer = new wxBoxSizer(wxHORIZONTAL);
	sizer->Add(buttons.log, 1, wxEXPAND);
	sizer->Add(buttons.base, 1, wxEXPAND);
	sizer->Add(buttons.config, 1, wxEXPAND);
	sizer->Add(buttons.prcxml, 1, wxEXPAND);
	lBagSizer->Add(sizer, wxGBPosition(7, 0), wxGBSpan(1, 6), wxEXPAND);

	lBagSizer->Add(logWindow, wxGBPosition(8, 0), wxGBSpan(4, 6), wxEXPAND);

	// Image Stuff
	wxFont* boldFont = new wxFont();
	boldFont->SetWeight(wxFONTWEIGHT_BOLD);

	initPreview.chara_1 = new wxStaticBitmap(rPanel, wxID_ANY, wxBitmap());
	initPreview.chara_1->SetToolTip("chara_1");
	initPreview.chara_2 = new wxStaticBitmap(rPanel, wxID_ANY, wxBitmap());
	initPreview.chara_2->SetToolTip("chara_2");
	initPreview.chara_4 = new wxStaticBitmap(rPanel, wxID_ANY, wxBitmap());
	initPreview.chara_4->SetToolTip("chara_4");
	initPreview.chara_7 = new wxStaticBitmap(rPanel, wxID_ANY, wxBitmap());
	initPreview.chara_7->SetToolTip("chara_7");
	initPreview.text = new wxStaticText(rPanel, wxID_ANY, "Old Slot (Vanilla)");
	initPreview.text->SetFont(*boldFont);

	finalPreview.chara_1 = new wxStaticBitmap(rPanel, wxID_ANY, wxBitmap());
	finalPreview.chara_1->SetToolTip("chara_1: Double click to toggle gamma correction.");
	finalPreview.chara_1->Bind(wxEVT_LEFT_DCLICK, &MainFrame::onGammaPressed, this);
	finalPreview.chara_2 = new wxStaticBitmap(rPanel, wxID_ANY, wxBitmap());
	finalPreview.chara_2->SetToolTip("chara_2: Double click to toggle gamma correction.");
	finalPreview.chara_2->Bind(wxEVT_LEFT_DCLICK, &MainFrame::onGammaPressed, this);
	finalPreview.chara_4 = new wxStaticBitmap(rPanel, wxID_ANY, wxBitmap());
	finalPreview.chara_4->SetToolTip("chara_4: Double click to toggle gamma correction.");
	finalPreview.chara_4->Bind(wxEVT_LEFT_DCLICK, &MainFrame::onGammaPressed, this);
	finalPreview.chara_7 = new wxStaticBitmap(rPanel, wxID_ANY, wxBitmap());
	finalPreview.chara_7->SetToolTip("chara_7: Double click to toggle gamma correction.");
	finalPreview.chara_7->Bind(wxEVT_LEFT_DCLICK, &MainFrame::onGammaPressed, this);
	finalPreview.text = new wxStaticText(rPanel, wxID_ANY, "New Slot (Mod)");
	finalPreview.text->SetFont(*boldFont);


	int files = 0;
	if (fs::exists("Files/textures/vanilla"))
	{
		for (auto i : fs::directory_iterator("Files/textures/vanilla"))
		{
			if (i.is_directory())
			{
				for (auto j : fs::directory_iterator(i))
				{
					if (j.path().extension() == ".png")
					{
						files++;
					}
				}
			}
		}
	}
	if (files < 2266)
	{
		settings.preview = false;
		updateSettings();

		previewMenu->SetHelp("Missing textures: Files/textures/vanilla has " + to_string(files) + "/2266 files.");
		previewMenu->Enable(false);
		previewMenu->Check(false);
	}
	
	if (settings.preview)
	{
		updateBitmap(initPreview.chara_1, "Files/textures/vanilla/chara_1/chara_1_random_00.png", 345, 345);
		updateBitmap(initPreview.chara_2, "Files/textures/vanilla/chara_2/chara_2_random_00.png", 64, 64);
		updateBitmap(initPreview.chara_4, "Files/textures/vanilla/chara_4/chara_4_random_00.png", 100, 100);
		updateBitmap(initPreview.chara_7, "Files/textures/vanilla/chara_7/chara_7_random_00.png", 176, 117);

		updateBitmap(finalPreview.chara_1, "Files/textures/vanilla/chara_1/chara_1_random_00.png", 345, 345);
		updateBitmap(finalPreview.chara_2, "Files/textures/vanilla/chara_2/chara_2_random_00.png", 64, 64);
		updateBitmap(finalPreview.chara_4, "Files/textures/vanilla/chara_4/chara_4_random_00.png", 100, 100);
		updateBitmap(finalPreview.chara_7, "Files/textures/vanilla/chara_7/chara_7_random_00.png", 176, 117);
	}

	rBagSizer->Add(initPreview.chara_7, wxGBPosition(0, 0), wxGBSpan(1, 1), wxEXPAND);
	rBagSizer->Add(initPreview.chara_4, wxGBPosition(0, 1), wxGBSpan(1, 1), wxEXPAND);
	rBagSizer->Add(initPreview.chara_2, wxGBPosition(0, 2), wxGBSpan(1, 1), wxEXPAND);
	rBagSizer->Add(initPreview.chara_1, wxGBPosition(1, 0), wxGBSpan(1, 3), wxEXPAND);
	rBagSizer->Add(initPreview.text, wxGBPosition(2, 0), wxGBSpan(1, 3), wxALIGN_CENTER_HORIZONTAL);

	rBagSizer->Add(finalPreview.chara_7, wxGBPosition(0, 3), wxGBSpan(1, 1), wxEXPAND);
	rBagSizer->Add(finalPreview.chara_4, wxGBPosition(0, 4), wxGBSpan(1, 1), wxEXPAND);
	rBagSizer->Add(finalPreview.chara_2, wxGBPosition(0, 5), wxGBSpan(1, 1), wxEXPAND);
	rBagSizer->Add(finalPreview.chara_1, wxGBPosition(1, 3), wxGBSpan(1, 3), wxEXPAND);
	rBagSizer->Add(finalPreview.text, wxGBPosition(2, 3), wxGBSpan(1, 3), wxALIGN_CENTER_HORIZONTAL);

	splitter->SplitVertically(lPanel, rPanel);
	// splitter->SetMinimumPaneSize(0);
	splitter->SetSashGravity(0.5);

	lBagSizer->AddGrowableCol(0);
	lBagSizer->AddGrowableCol(1);
	lBagSizer->AddGrowableCol(2);
	lBagSizer->AddGrowableCol(3);
	lBagSizer->AddGrowableCol(4);
	lBagSizer->AddGrowableCol(5);

	lBagSizer->AddGrowableRow(8);

	wxBoxSizer* lVSizer = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer* lHSizer = new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer* rVSizer = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer* rHSizer = new wxBoxSizer(wxHORIZONTAL);

	lVSizer->Add(lHSizer, 1, wxALIGN_CENTER_HORIZONTAL | wxALL, 20);
	lHSizer->Add(lBagSizer, 1, wxALIGN_CENTER_VERTICAL);
	rVSizer->Add(rHSizer, 1, wxALIGN_CENTER_HORIZONTAL | wxALL, 20);
	rHSizer->Add(rBagSizer, 1, wxALIGN_CENTER_VERTICAL);

	lPanel->SetSizerAndFit(lVSizer);
	rPanel->SetSizerAndFit(rVSizer);

	if (!settings.preview)
	{
		rPanel->Hide();

		initPreview.chara_1->Hide();
		initPreview.chara_2->Hide();
		initPreview.chara_4->Hide();
		initPreview.chara_7->Hide();
		finalPreview.text->Hide();

		finalPreview.chara_1->Hide();
		finalPreview.chara_2->Hide();
		finalPreview.chara_4->Hide();
		finalPreview.chara_7->Hide();
		finalPreview.text->Hide();

		splitter->Unsplit();
	}

	sizerM = new wxBoxSizer(wxVERTICAL);
	sizerM->Add(splitter, 1, wxEXPAND);
	this->SetSizerAndFit(sizerM);

	logWindow->Show(false);
	lPanel->SendSizeEvent();
}

/* --- HELPER FUNCTIONS --- */
void MainFrame::updateControls(bool character, bool fileType, bool initSlot, bool finalSlot, bool newInkSlot)
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
		codesFilled = true;

		// Enable or disable file type checkbox based on whether or not it exists in character's map
		wxArrayString selectablefileTypes = mHandler.wxGetFileTypes(codes, settings.selectionType);
		auto allFileTypes = mHandler.wxGetFileTypes();
		for (int i = 0, j = 0; i < allFileTypes.size(); i++)
		{
			if (j < selectablefileTypes.size() && selectablefileTypes[j] == allFileTypes[i])
			{
				fileTypeBoxes[i]->Enable();
				if (settings.selectAllByDefault)
				{
					fileTypeBoxes[i]->SetValue(true);
				}
				j++;
			}
			else
			{
				fileTypeBoxes[i]->Disable();
				fileTypeBoxes[i]->SetValue(false);
			}
		}

		initSlot = true;
	}

	if (initSlot)
	{
		if (!codesFilled)
		{
			codes = getSelectedCharCodes();
			codesFilled = true;
		}
		if (!fileTypesFilled)
		{
			fileTypes = getSelectedFileTypes();
			fileTypesFilled = true;
		}

		string oldSlot = (initSlots.list->GetSelection() != wxNOT_FOUND) ? initSlots.list->GetStringSelection().ToStdString() : "";

		if (!fileTypes.empty())
		{
			initSlots.list->Set(mHandler.wxGetSlots(codes, fileTypes, settings.selectionType));
		}
		else
		{
			initSlots.list->Clear();
		}

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

			Slot slot(initSlots.list->GetStringSelection().ToStdString());

			if (settings.preview)
			{
				if (!codes.empty() && slot.getInt() != 999 && codes[0] != "element")
				{
					string code = codes[0].ToStdString();

					if (code == "eflame")
					{
						code = "eflame_first";
					}
					else if (code == "elight")
					{
						code = "elight_first";
					}

					string arr[] = { "1", "2", "4", "7" };
					for (int i = 0; i < 4; i++)
					{
						if (i == 3 && code == "elight_first")
						{
							code = "eflame_first";
						}

						string path1 = mHandler.getPath() + "/ui/replace/chara/chara_" + arr[i] + "/chara_" + arr[i] + "_" + code + "_" + (i == 3 ? "00" : slot.getString());
						string path2 = mHandler.getPath() + "/ui/replace_patch/chara/chara_" + arr[i] + "/chara_" + arr[i] + "_" + code + "_" + (i == 3 ? "00" : slot.getString());
						string path3 = "";
						string path4 = "";
						string codeT;
						string command;

						if (code == "eflame_first" || code == "elight_first")
						{
							codeT = code.substr(0, 7) + "only";

							path3 = mHandler.getPath() + "/ui/replace/chara/chara_" + arr[i] + "/chara_" + arr[i] + "_" + codeT + "_" + (i == 3 ? "00" : slot.getString());
							path4 = mHandler.getPath() + "/ui/replace_patch/chara/chara_" + arr[i] + "/chara_" + arr[i] + "_" + codeT + "_" + (i == 3 ? "00" : slot.getString());
						}

						if (fs::exists(path1 + ".bntx"))
						{
							wxExecute("Files/textures/ultimate_tex_cli.exe \"" + path1 + ".bntx\" " +
								"\"" + "Files/textures/modded/chara_" + arr[i] + "/chara_" + arr[i] + "_" + code +
								"_" + (i == 3 ? "00" : slot.getString()) + ".png\"", wxEXEC_SYNC | wxEXEC_NODISABLE | wxEXEC_HIDE_CONSOLE);
						}
						else if (fs::exists(path2 + ".bntx"))
						{
							wxExecute("Files/textures/ultimate_tex_cli.exe \"" + path2 + ".bntx\" " +
								"\"" + "Files/textures/modded/chara_" + arr[i] + "/chara_" + arr[i] + "_" + code +
								"_" + (i == 3 ? "00" : slot.getString()) + ".png\"", wxEXEC_SYNC | wxEXEC_NODISABLE | wxEXEC_HIDE_CONSOLE);
						}
						else if (!path3.empty() && fs::exists(path3 + ".bntx"))
						{
							wxExecute("Files/textures/ultimate_tex_cli.exe \"" + path3 + ".bntx\" " +
								"\"" + "Files/textures/modded/chara_" + arr[i] + "/chara_" + arr[i] + "_" + code +
								"_" + (i == 3 ? "00" : slot.getString()) + ".png\"", wxEXEC_SYNC | wxEXEC_NODISABLE | wxEXEC_HIDE_CONSOLE);
						}
						else if (!path4.empty() && fs::exists(path4 + ".bntx"))
						{
							wxExecute("Files/textures/ultimate_tex_cli.exe \"" + path4 + ".bntx\" " +
								"\"" + "Files/textures/modded/chara_" + arr[i] + "/chara_" + arr[i] + "_" + code +
								"_" + (i == 3 ? "00" : slot.getString()) + ".png\"", wxEXEC_SYNC | wxEXEC_NODISABLE | wxEXEC_HIDE_CONSOLE);
						}
						else
						{
							string code2 = code;
							Slot sSlot(slot);

							if (fs::exists("Files/textures/modded/chara_" + arr[i] + "/chara_" + arr[i] + "_" + code + "_" + (i == 3 ? "00" : slot.getString()) + ".png"))
							{
								fs::remove("Files/textures/modded/chara_" + arr[i] + "/chara_" + arr[i] + "_" + code + "_" + (i == 3 ? "00" : slot.getString()) + ".png");
							}

							if (slot.getInt() > 7)
							{
								if (baseUpToDate)
								{
									sSlot = mHandler.getBaseSlot(codes[0].ToStdString(), slot);
								}
								else
								{
									code2 = (i == 3 && !mHandler.getName(code).empty()) ? code : "random";
									sSlot = Slot(0);
								}
							}
							else if (mHandler.getName(code).empty())
							{
								code2 = "random";
								sSlot = Slot(0);
							}

							fs::copy("Files/textures/vanilla/chara_" + arr[i] + "/chara_" + arr[i] + "_" + code2 + "_" + (i == 3 ? "00" : sSlot.getString()) + ".png",
								"Files/textures/modded/chara_" + arr[i] + "/chara_" + arr[i] + "_" + code + "_" + (i == 3 ? "00" : slot.getString()) + ".png");
						}
					}

					code = codes[0].ToStdString();

					if (code == "eflame")
					{
						code = "eflame_first";
					}
					else if (code == "elight")
					{
						code = "elight_first";
					}

					updateBitmap(finalPreview.chara_1, "Files/textures/modded/chara_1/chara_1_" + code + "_" + slot.getString() + ".png", 345, 345);
					updateBitmap(finalPreview.chara_2, "Files/textures/modded/chara_2/chara_2_" + code + "_" + slot.getString() + ".png", 64, 64);
					updateBitmap(finalPreview.chara_4, "Files/textures/modded/chara_4/chara_4_" + code + "_" + slot.getString() + ".png", 100, 100);
					if (code == "elight_first")
					{
						code = "eflame_first";
					}
					updateBitmap(finalPreview.chara_7, "Files/textures/modded/chara_7/chara_7_" + code + "_00.png", 176, 117);
				}
				else
				{
					updateBitmap(finalPreview.chara_1, "Files/textures/vanilla/chara_1/chara_1_random_00.png", 345, 345);
					updateBitmap(finalPreview.chara_2, "Files/textures/vanilla/chara_2/chara_2_random_00.png", 64, 64);
					updateBitmap(finalPreview.chara_4, "Files/textures/vanilla/chara_4/chara_4_random_00.png", 100, 100);
					updateBitmap(finalPreview.chara_7, "Files/textures/vanilla/chara_7/chara_7_random_00.png", 176, 117);
				}
			}
		}
		else
		{
			if (settings.preview)
			{
				updateBitmap(finalPreview.chara_1, "Files/textures/vanilla/chara_1/chara_1_random_00.png", 345, 345);
				updateBitmap(finalPreview.chara_2, "Files/textures/vanilla/chara_2/chara_2_random_00.png", 64, 64);
				updateBitmap(finalPreview.chara_4, "Files/textures/vanilla/chara_4/chara_4_random_00.png", 100, 100);
				updateBitmap(finalPreview.chara_7, "Files/textures/vanilla/chara_7/chara_7_random_00.png", 176, 117);
			}

			buttons.mov->Disable();
			buttons.dup->Disable();
			buttons.del->Disable();
		}

		finalSlot = true;
	}

	if (finalSlot)
	{
		if (!codesFilled)
		{
			codes = getSelectedCharCodes();
			codesFilled = true;
		}
		if (!fileTypesFilled)
		{
			fileTypes = getSelectedFileTypes();
			fileTypesFilled = true;
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

		Slot slot(finalSlots.list->GetValue());

		if (settings.preview)
		{
			if (!codes.empty() && !mHandler.getName(codes[0].ToStdString()).empty() && slot.getInt() <= 7)
			{
				string code = codes[0].ToStdString();

				if (code == "eflame")
				{
					code = "eflame_first";
				}
				else if (code == "elight")
				{
					code = "elight_first";
				}
				else if (code == "element")
				{
					code = "random";
				}

				updateBitmap(initPreview.chara_1, "Files/textures/vanilla/chara_1/chara_1_" + code + "_" + slot.getString() + ".png", 345, 345);
				updateBitmap(initPreview.chara_2, "Files/textures/vanilla/chara_2/chara_2_" + code + "_" + slot.getString() + ".png", 64, 64);
				updateBitmap(initPreview.chara_4, "Files/textures/vanilla/chara_4/chara_4_" + code + "_" + slot.getString() + ".png", 100, 100);
				if (code == "elight_first")
				{
					code = "eflame_first";
				}
				updateBitmap(initPreview.chara_7, "Files/textures/vanilla/chara_7/chara_7_" + code + "_00.png", 176, 117);
			}
			else
			{
				updateBitmap(initPreview.chara_1, "Files/textures/vanilla/chara_1/chara_1_random_00.png", 345, 345);
				updateBitmap(initPreview.chara_2, "Files/textures/vanilla/chara_2/chara_2_random_00.png", 64, 64);
				updateBitmap(initPreview.chara_4, "Files/textures/vanilla/chara_4/chara_4_random_00.png", 100, 100);
				updateBitmap(initPreview.chara_7, "Files/textures/vanilla/chara_7/chara_7_random_00.png", 176, 117);
			}
		}
	}

	if (!baseUpToDate)
	{
		buttons.base->Show();
		buttons.config->Hide();
		buttons.prcxml->Hide();
	}
	else
	{
		buttons.base->Hide();
		buttons.config->Show();
		buttons.prcxml->Show();
	}

	if (!mHandler.getPath().empty())
	{
		if (newInkSlot)
		{
			inkMenu->Enable(false);
			inkMenu->SetHelp("Select base slots to enable this feature.");
		}
		else if (!mHandler.hasAddSlot("inkling"))
		{
			inkMenu->Enable();
			inkMenu->SetHelp("Add or modify colors. Required for additional slots.");
		}

		cssMenu->Enable();
		cssMenu->SetHelp("Renames ui files' character code to selected code. Useful for CSS Addition.");
	}
	else
	{
		buttons.base->Hide();
		buttons.config->Hide();
		buttons.prcxml->Hide();

		inkMenu->Enable(false);
		inkMenu->SetHelp("Open a directory to enable this feature.");

		cssMenu->Enable(false);
		cssMenu->SetHelp("Open a directory to enable this feature.");
	}

	lPanel->SendSizeEvent();
}

void MainFrame::updateBitmap(wxStaticBitmap* sBitmap, string path, int width, int height, bool gammaFix)
{
	try
	{
		wxImage image = wxBitmap(path, wxBITMAP_TYPE_PNG).ConvertToImage().Scale(width, height);
		int size = image.GetHeight() * image.GetWidth();

		if (gammaFix)
		{
			unsigned char* data = image.GetData();
			for (unsigned int i = 0; i < size; i++)
			{
				data[0] = pow(data[0] / 255.0f, 1.0f / 2.2f) * 255.0f;
				data[1] = pow(data[1] / 255.0f, 1.0f / 2.2f) * 255.0f;
				data[2] = pow(data[2] / 255.0f, 1.0f / 2.2f) * 255.0f;

				data += 3;
			}
		}

		sBitmap->SetBitmap(image);
		sBitmap->SetName(path);
	}
	catch (...)
	{
		log->LogText("> Error: Could not update UI for " + path);
	}
}

void MainFrame::readSettings()
{
	ifstream settingsFile("Files/settings.data");
	if (settingsFile.is_open())
	{
		int bit;

		settingsFile >> bit;
		settings.selectAllByDefault = (bit == 1) ? true : false;
		settingsFile >> bit;
		settings.preview = (bit == 1) ? true : false;
		settingsFile >> bit;
		settings.baseSource = (bit == 1) ? true : false;
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
	ofstream settingsFile("Files/settings.data");
	if (settingsFile.is_open())
	{
		settingsFile << settings.selectAllByDefault << ' ';
		settingsFile << settings.preview << ' ';
		settingsFile << settings.baseSource << ' ';
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
	else if (setting == "baseSource")
	{
		settings.baseSource = !settings.baseSource;
		if (settings.baseSource)
		{
			log->LogText("> Base Slots for c00-c07 are selectable.");
			baseUpToDate = false;
		}
		else
		{
			log->LogText("> Base Slots for c00-c07 are NOT selectable.");
			baseUpToDate = !mHandler.hasAddSlot();
		}
		updateControls();
	}
	else if (setting == "preview")
	{
		settings.preview = !settings.preview;
		updateSettings();

		wxExecute(wxFileName(wxStandardPaths::Get().GetExecutablePath()).GetPath() + "/" + exe);
		Close();
	}
	else if (setting == "selectAllByDefault")
	{
		settings.selectAllByDefault = !settings.selectAllByDefault;
		if (settings.selectAllByDefault)
		{
			log->LogText("> All available file types will be selected by default on selection.");
		}
		else
		{
			log->LogText("> No file types will be selected by default on selection.");
		}
	}

	updateSettings();
}

void MainFrame::onBrowse(wxCommandEvent& evt)
{
	string path = "";

	if (static_cast<wxArgument*>(evt.GetEventUserData())->str == "text")
	{
		path = browse.text->GetValue().ToStdString();

		if (!path.empty() && path[0] == '"' && path[path.size() - 1] == '"')
		{
			path = path.substr(1);

			if (!path.empty())
			{
				path = path.substr(0, path.size() - 1);
			}
		}

		if (!fs::is_directory(path))
		{
			path = "";
		}
		else if (path[path.size() - 1] == '/' || path[path.size() - 1] == '\\')
		{
			path = path.substr(0, path.size() - 1);
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
		try
		{
			mHandler.readFiles(path);
		}
		catch (...)
		{
			log->LogText("> Error: Unable to read files correctly.");
		}

		browse.text->SetValue(mHandler.getPath());
		if (settings.baseSource)
		{
			baseUpToDate = false;
		}
		else
		{
			baseUpToDate = !mHandler.hasAddSlot();
		}
		updateControls(true, true, true, true, mHandler.hasAddSlot("inkling"));
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
		if (settings.baseSource)
		{
			baseUpToDate = false;
		}
		else
		{
			baseUpToDate = !mHandler.hasAddSlot();
		}
		updateControls(false, false, true, true, finalSlots.list->GetValue() > 7 && inkHasSlot);
	}
	else
	{
		updateControls(mHandler.getNumCharacters() != arg->num, true, true, true);
	}
}

void MainFrame::onBatchPressed(wxCommandEvent& evt)
{
	wxDirDialog dialog(this, "Choose the directory containing multiple mod folders...", "", wxDD_DEFAULT_STYLE | wxDD_DIR_MUST_EXIST);
	if (dialog.ShowModal() != wxID_CANCEL)
	{
		ModHandler mod(log);
		int count = 0;

		for (const auto& modFolder : fs::directory_iterator(dialog.GetPath().ToStdString()))
		{
			if (modFolder.is_directory())
			{
				mod.readFiles(modFolder.path().string());

				if (mod.hasChar())
				{
					BatchSelection(this, wxID_ANY, "Batch Job #" + count, &mod, settings).ShowModal();
					count++;
				}
			}
		}

		wxMessageBox("Success! " + to_string(count) + " mod folders discovered!", "Finished", wxOK);
	}
}

void MainFrame::onLogPressed(wxCommandEvent& evt)
{
	if (logWindow->IsShown())
	{
		logWindow->Show(false);
		buttons.log->SetLabel("Show Log");
	}
	else
	{
		logWindow->Show(true);
		buttons.log->SetLabel("Hide Log");
	}

	lPanel->SendSizeEvent();
}

void MainFrame::onBasePressed(wxCommandEvent& evt)
{
	BaseSelection dlg(this, wxID_ANY, "Choose Base Slots", &mHandler, settings);

	if (dlg.ShowModal() == wxID_OK)
	{
		mHandler.setBaseSlots(dlg.getBaseSlots());

		buttons.base->Hide();
		buttons.config->Show();
		buttons.prcxml->Show();

		inkMenu->Enable();
		inkMenu->SetHelp("Add or modify colors. Required for additional slots.");

		baseUpToDate = true;

		lPanel->SendSizeEvent();
	}
}

void MainFrame::onConfigPressed(wxCommandEvent& evt)
{
	mHandler.create_config();
}

void MainFrame::onPrcPressed(wxCommandEvent& evt)
{
	onPrcPressed();
}

void MainFrame::onPrcPressed(map<string, map<int, CssData>>* css)
{
	if (mHandler.hasChar())
	{
		PrcSelection dlg(this, wxID_ANY, "Make Selection", &mHandler, settings, css);
		if (dlg.ShowModal() == wxID_OK)
		{
			auto maxSlots = dlg.getMaxSlots();
			auto cIndex = dlg.getDB("cIndex");
			auto nIndex = dlg.getDB("nIndex");
			auto cGroup = dlg.getDB("cGroup");
			auto finalAnnouncers = dlg.getAnnouncers();

			if (!maxSlots.empty() || !cIndex.empty() || !nIndex.empty() || !cGroup.empty() || !finalAnnouncers.empty())
			{
				mHandler.create_db_prcxml(cIndex, nIndex, cGroup, maxSlots, finalAnnouncers);
			}
			else if (fs::exists(mHandler.getPath() + "/ui/param/database/ui_chara_db.prcxml"))
			{
				fs::remove(mHandler.getPath() + "/ui/param/database/ui_chara_db.prcxml");
				log->LogText("> WARN: ui_chara_db.prcxml is not needed, previous one was deleted to avoid issues.");
			}
			else
			{
				log->LogText("> NOTE: ui_chara_db.prcxml is not needed.");
			}

			auto finalNames = dlg.getNames();
			if (!finalNames.empty())
			{
				mHandler.create_message_xmsbt(finalNames);
			}
			else if (fs::exists(mHandler.getPath() + "/ui/message/msg_name.xmsbt"))
			{
				fs::remove(mHandler.getPath() + "/ui/message/msg_name.xmsbt");
				if (fs::is_empty(mHandler.getPath() + "/ui/message"))
				{
					fs::remove(mHandler.getPath() + "/ui/message");
				}

				log->LogText("> WARN: msg_name.xmsbt is not needed, previous one was deleted to avoid issues.");
			}
		}
	}
	else
	{
		log->LogText("> NOTE: Mod is empty, cannot create a prcxml!");
	}
}

void MainFrame::onInkPressed(wxCommandEvent& evt)
{
	InkSelection dlg(this, wxID_ANY, "Choose Inkling Colors", &mHandler, settings);
	if (dlg.ShowModal() == wxID_OK)
	{
		auto finalColors = dlg.getFinalColors();

		if (!finalColors.empty())
		{
			mHandler.create_ink_prcxml(finalColors);
		}
		else
		{
			log->LogText("> N/A: No changes were made.");
		}
	}
}

void MainFrame::onCSSPressed(wxCommandEvent& evt)
{
	CssRedirectSelection dlg(this, wxID_ANY, "Redirect CSS", &mHandler);
	if (dlg.ShowModal() == wxID_OK)
	{
		auto res = dlg.getChanges();

		// Rename UI stuff
		for (auto i = res.begin(); i != res.end(); i++)
		{
			for (auto j = i->second.begin(); j != i->second.end(); j++)
			{
				for (int k = 0; k < 12; k++)
				{
					if (k == 7)
					{
						string oldF = mHandler.getPath() + "/ui/replace/chara/chara_7/chara_7_" + i->first + "_00.bntx";

						if (fs::exists(oldF))
						{
							string newF = mHandler.getPath() + "/ui/replace/chara/chara_7/chara_7_" + j->second.code + "_00.bntx";
							fs::rename(oldF, newF);
						}
					}
					else
					{
						for (int l = 0; l < j->second.color_num; l++)
						{
							string oldF = mHandler.getPath() + "/ui/replace/chara/chara_" + to_string(k) + "/chara_" + to_string(k) + "_" + i->first + "_" + Slot(l + j->second.color_start_index).getString() + ".bntx";

							if (fs::exists(oldF))
							{
								string newF = mHandler.getPath() + "/ui/replace/chara/chara_" + to_string(k) + "/chara_" + to_string(k) + "_" + j->second.code + "_" + Slot(l).getString() + ".bntx";
								fs::rename(oldF, newF);
							}
						}
					}
				}
			}
		}


		auto oldBase = mHandler.getBaseSlots();
		mHandler.readFiles(mHandler.getPath());
		mHandler.setCssRedirects(dlg.getRedirects());
		mHandler.setBaseSlots(oldBase);
		updateControls(true, true, true, true, false);

		onPrcPressed(&res);
	}
}

void MainFrame::onGammaPressed(wxMouseEvent& event)
{
	gammaFix = !gammaFix;
	
	updateBitmap(finalPreview.chara_1, finalPreview.chara_1->GetName().ToStdString(), 345, 345, gammaFix);
	updateBitmap(finalPreview.chara_2, finalPreview.chara_2->GetName().ToStdString(), 64, 64, gammaFix);
	updateBitmap(finalPreview.chara_4, finalPreview.chara_4->GetName().ToStdString(), 100, 100, gammaFix);
	updateBitmap(finalPreview.chara_7, finalPreview.chara_7->GetName().ToStdString(), 176, 117, gammaFix);
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
		string code = mHandler.getCode(charsList->GetString(selection).ToStdString());
		codes.Add(code.empty() ? charsList->GetString(selection).ToStdString() : code);
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
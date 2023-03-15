#include "App.h"
#include "MainFrame.h"
#include <wx/wx.h>

wxIMPLEMENT_APP(App);

bool App::OnInit()
{
	MainFrame* mainFrame = new MainFrame("Smash Ultimate Mod Helper");
	mainFrame->SetIcons(wxICON(SMASH_ICON));
	mainFrame->SetClientSize(400, 300);
	mainFrame->SetMinSize(mainFrame->GetSize());
	mainFrame->Show();
	
	return true;
}
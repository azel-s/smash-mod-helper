#pragma once

#include "VanillaHandler.h"
#include <unordered_map>
#include <unordered_set>
#include <map>
#include <set>
#include <wx/wx.h>
using json = nlohmann::json;
using std::vector, std::set, std::unordered_set, std::map, std::unordered_map;

class ModHandler : public VanillaHandler
{
private:
	string path; // Mod Path
	vector<string> fileTypes; // Supported file types

	// Access Order: code, filetype, slot, files
	unordered_map<string, unordered_map<string, map<Slot, set<Path>>>> files;
	// Access Order: code, final-slot, base-slot,
	unordered_map<string, map<Slot, Slot>> slots;
	// Access Order: new-code, old-code, base-slot, final-slot
	map<string, map<string, map<Slot, Slot>>> css_redirects;

	wxLogTextCtrl* log;

	bool debug;

	/* --- HELPERS (UNIVERSAL) --- */
	void addFile(string code, string fileType, int slot, string file);
	void outputUTF(wofstream& file, wxString str, bool parse = false);

public:
	/* --- CONSTRUCTORS --- */
	ModHandler(wxLogTextCtrl* log = nullptr, string path = "");

	/* --- SETTERS (UNIVERSAL) --- */
	void setBaseSlots(map<string, map<Slot, set<Slot>>> slots);
	void setCssRedirects(map<string, map<string, map<Slot, Slot>>> redirects);
	void setDebug(bool debug);

	/* --- SETTERS (WX) --- */
	void wxSetLog(wxLogTextCtrl* log);

	/* --- GETTERS (UNIVERSAL) --- */
	string getPath();
	int getNumCharacters();
	map<string, set<Slot>> getAllSlots(bool withAll = true) const;
	set<Slot> getAddSlots(string code)  const;
	map<string, set<Slot>> getAddSlots() const;
	Slot getBaseSlot(string code, Slot slot) const;
	map<string, map<Slot, set<Slot>>> getBaseSlots() const;
	InklingColor getInklingColor(Slot slot);
	Name getMessage(string code, Slot slot) const;
	Slot getMaxSlot(string code) const;
	string getRedirectCode(string newCode);
	Slot getRedirectSlot(string newCode, Slot slot);

	/* --- GETTERS (WX) --- */
	wxArrayString wxGetCharacterNames(string fileType = "") const;
	wxArrayString wxGetFileTypes(string code = "") const;
	wxArrayString wxGetFileTypes(wxArrayString codes, bool findInAll = false) const;
	wxArrayString wxGetSlots(string code, wxArrayString fileTypes = {}, bool findInAll = false) const;
	wxArrayString wxGetSlots(wxArrayString codes, wxArrayString fileTypes = {}, bool findInAll = false) const;

	/* --- VERIFIERS (UNIVERSAL) ---*/
	bool hasChar(string code = "") const;
	bool hasFileType(string fileType = "") const;
	bool hasAddSlot(string code = "") const;
	bool isKirbyCopyOnly() const;
	bool isKirbyCopyOnly(Slot slot) const;

	/* --- VERIFIERS (WX) --- */
	bool wxHasSlot(string code, Slot slot, wxArrayString fileTypes = {}, bool findInAll = false) const;
	bool wxHasSlot(wxArrayString codes, Slot slot, wxArrayString fileTypes = {}, bool findInAll = false) const;

	/* --- FUNCTIONS (UNIVERSAL) --- */
	void readFiles(string path);
	void deleteEmptyDirs(string path);
	void deleteDesktopINI(string path);

	/* --- FUNCTIONS (WX) --- */
	void wxLog(string message, bool debug = true);
	void adjustFiles(string action, string code, wxArrayString fileTypes, Slot iSlot, Slot fSlot);
	void adjustFiles(string action, wxArrayString codes, wxArrayString fileTypes, Slot iSlot, Slot fSlot);

	/* --- FUNCTIONS (CONFIG/PRC) --- */
	Config getNewDirSlots();
	void getNewDirSlots(string code, Slot slot, Config& config);

	// Creators
	void create_config();
	void create_message_xmsbt(map<string, map<Slot, Name>>& names);
	void create_db_prcxml
	(
		map<string, map<Slot, int>>& cIndex,
		map<string, map<Slot, int>>& nIndex,
		map<string, map<Slot, int>>& cGroup,
		map<string, Slot>& maxSlots,
		map<string, map<int, Name>>& announcers
	);
	void create_ink_prcxml(map<Slot, InklingColor>& inklingColors);

	// Readers
	// og charcode, start_index, data
	map<string, map<int, CssData>> read_prcxml_css();
	map<string, map<Slot, Slot>> read_config_slots();
	map<string, map<Slot, Name>> read_message_names();
	map<Slot, InklingColor> read_ink_colors();
};
#include "CssRedirectSelection.h"
#include <wx/gbsizer.h>

void CssRedirectSelection::onAddL(wxCommandEvent& evt)
{
	string charcode = static_cast<wxCSSArgument*>(evt.GetEventUserData())->charcode;
	Node* node = static_cast<wxCSSArgument*>(evt.GetEventUserData())->node;
	wxFlexGridSizer* gridSizer = static_cast<wxCSSArgument*>(evt.GetEventUserData())->sizer;

	if (node->prev)
	{
		if (node->data->left.getInt() != node->data->right.getInt())
		{
			node->prev->data->addRight();
			node->data->addLeft();
		}
		else if (node->next)
		{
			node->next->data->subLeft();
			Node::deleteMid(node->prev, node, node->next);
		}
		else
		{
			node->prev->data->addRight();
			Node::deleteMid(node->prev, node, node->next);
		}
	}
	else if (node->data->left.getInt() == node->data->right.getInt())
	{
		if (node->next)
		{
			node->next->data->subLeft();
			Node::deleteMid(node->prev, node, node->next);
		}
	}

	this->SendSizeEvent();
}

void CssRedirectSelection::onAddR(wxCommandEvent& evt)
{
	string charcode = static_cast<wxCSSArgument*>(evt.GetEventUserData())->charcode;
	Node* node = static_cast<wxCSSArgument*>(evt.GetEventUserData())->node;
	wxFlexGridSizer* gridSizer = static_cast<wxCSSArgument*>(evt.GetEventUserData())->sizer;

	if (node->next)
	{
		if (node->next->data->left.getInt() != node->next->data->right.getInt())
		{
			node->next->data->addLeft();
			node->data->addRight();
		}
		else
		{
			node->data->addRight();
			Node::deleteMid(node, node->next, node->next->next);
		}
	}

	this->SendSizeEvent();
}

void CssRedirectSelection::onSubL(wxCommandEvent& evt)
{
	string charcode = static_cast<wxCSSArgument*>(evt.GetEventUserData())->charcode;
	Node* node = static_cast<wxCSSArgument*>(evt.GetEventUserData())->node;
	wxFlexGridSizer* gridSizer = static_cast<wxCSSArgument*>(evt.GetEventUserData())->sizer;

	if (node->prev)
	{
		if (node->prev->data->left.getInt() != node->prev->data->right.getInt())
		{
			node->prev->data->subRight();
			node->data->subLeft();
		}
		else
		{
			node->data->subLeft();
			Node::deleteMid(node->prev->prev, node->prev, node);
		}
	}

	this->SendSizeEvent();
}

void CssRedirectSelection::onSubR(wxCommandEvent& evt)
{
	string code = static_cast<wxCSSArgument*>(evt.GetEventUserData())->charcode;
	Node* node = static_cast<wxCSSArgument*>(evt.GetEventUserData())->node;
	wxFlexGridSizer* gridSizer = static_cast<wxCSSArgument*>(evt.GetEventUserData())->sizer;

	if (node->next)
	{
		if (node->data->left.getInt() != node->data->right.getInt())
		{
			node->next->data->subLeft();
			node->data->subRight();
		}
		else if (node->prev)
		{
			node->next->data->subLeft();
			Node::deleteMid(node->prev, node, node->next);
		}
	}
	else if (node->data->left.getInt() == node->data->right.getInt())
	{
		if (node->prev)
		{
			node->prev->data->addRight();
			Node::deleteMid(node->prev, node, node->next);
		}
	}
	else
	{
		node->data->subRight();

		wxTextCtrl* charcode = new wxTextCtrl(panel, wxID_ANY, "charcode", wxDefaultPosition, wxDefaultSize, wxTE_CENTRE);
		gridSizer->Add(charcode, 1, wxALIGN_CENTER | wxRIGHT, 20);

		wxButton* subL = new wxButton(panel, wxID_ANY, "-", wxDefaultPosition, FromDIP(wxSize(40, -1)));
		gridSizer->Add(subL, 1, wxALIGN_CENTER);

		wxButton* addL = new wxButton(panel, wxID_ANY, "+", wxDefaultPosition, FromDIP(wxSize(40, -1)));
		gridSizer->Add(addL, 1, wxALIGN_CENTER | wxRIGHT, 5);

		Slot l = Slot(node->data->right.getInt() + 1);
		Slot r = node->next ? Slot(node->next->data->left.getInt() - 1) : mHandler->getMaxSlot(code);

		wxStaticText* text = new wxStaticText(panel, wxID_ANY, "c" + l.getString() + " - c" + r.getString());
		gridSizer->Add(text, 1, wxALIGN_CENTER | wxRIGHT, 5);

		wxButton* subR = new wxButton(panel, wxID_ANY, "-", wxDefaultPosition, FromDIP(wxSize(40, -1)));
		gridSizer->Add(subR, 1, wxALIGN_CENTER);

		wxButton* addR = new wxButton(panel, wxID_ANY, "+", wxDefaultPosition, FromDIP(wxSize(40, -1)));
		gridSizer->Add(addR, 1, wxALIGN_CENTER);

		node->next = new Node(new Data(l, r, text, charcode, subL, subR, addL, addR), node, node->next);

		subL->Bind(wxEVT_BUTTON, &CssRedirectSelection::onSubL, this, wxID_ANY, wxID_ANY, new wxCSSArgument(code, node->next, gridSizer));
		addL->Bind(wxEVT_BUTTON, &CssRedirectSelection::onAddL, this, wxID_ANY, wxID_ANY, new wxCSSArgument(code, node->next, gridSizer));
		subR->Bind(wxEVT_BUTTON, &CssRedirectSelection::onSubR, this, wxID_ANY, wxID_ANY, new wxCSSArgument(code, node->next, gridSizer));
		addR->Bind(wxEVT_BUTTON, &CssRedirectSelection::onAddR, this, wxID_ANY, wxID_ANY, new wxCSSArgument(code, node->next, gridSizer));
	}

	this->SendSizeEvent();
}

CssRedirectSelection::CssRedirectSelection(wxWindow* parent, wxWindowID id,
	const wxString& title,
	ModHandler* mHandler,
	const wxPoint& pos,
	const wxSize& size,
	long style,
	const wxString& name) :
	wxDialog(parent, id, title, pos, size, style, name)
{
	this->mHandler = mHandler;
	auto slots = mHandler->getAllSlots(false);
	for (auto i = slots.begin(); i != slots.end(); i++)
	{
		if (mHandler->getName(i->first).empty())
		{
			slots.extract(i->first);
		}
	}

	auto css = mHandler->read_prcxml_css();

	wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
	wxFlexGridSizer* gridSizer;

	panel = new wxScrolled<wxPanel>(this, wxID_ANY, wxDefaultPosition, wxDefaultSize);
	panel->SetScrollRate(0, 10);

	wxFont* boldFont = new wxFont();
	boldFont->SetWeight(wxFONTWEIGHT_BOLD);

	for (auto i = slots.begin(); i != slots.end(); i++)
	{
		int index = 0;
		gridSizer = new wxFlexGridSizer(6, 10, 0);

		auto name = new wxStaticText(panel, wxID_ANY, mHandler->getName(i->first));
		name->SetFont(*boldFont);
		sizer->Add(name, 0, wxALIGN_CENTER_HORIZONTAL | wxBOTTOM, FromDIP(5));

		auto cssIter = css.find(i->first);
		if (cssIter != css.end())
		{
			bool first = true;

			Node* prev = nullptr;
			Node* current = nullptr;
			Node* next = nullptr;

			for (auto j = cssIter->second.begin(); j != cssIter->second.end(); j++)
			{
				wxTextCtrl* charcode;

				if (first)
				{
					charcode = new wxTextCtrl(panel, wxID_ANY, j->second.code, wxDefaultPosition, wxDefaultSize, wxTE_READONLY | wxTE_CENTRE);
				}
				else
				{
					charcode = new wxTextCtrl(panel, wxID_ANY, j->second.code, wxDefaultPosition, wxDefaultSize, wxTE_CENTRE);
				}

				gridSizer->Add(charcode, 1, wxALIGN_CENTER | wxRIGHT, 20);

				wxButton* subL = new wxButton(panel, wxID_ANY, "-", wxDefaultPosition, FromDIP(wxSize(40, -1)));
				gridSizer->Add(subL, 1, wxALIGN_CENTER);

				wxButton* addL = new wxButton(panel, wxID_ANY, "+", wxDefaultPosition, FromDIP(wxSize(40, -1)));
				gridSizer->Add(addL, 1, wxALIGN_CENTER | wxRIGHT, 5);

				Slot l(j->second.color_start_index);
				Slot r(j->second.color_start_index + (j->second.color_num - 1));

				wxStaticText* text = new wxStaticText(panel, wxID_ANY, "c" + l.getString() + " - c" + r.getString());
				gridSizer->Add(text, 1, wxALIGN_CENTER | wxRIGHT, 5);

				wxButton* subR = new wxButton(panel, wxID_ANY, "-", wxDefaultPosition, FromDIP(wxSize(40, -1)));
				gridSizer->Add(subR, 1, wxALIGN_CENTER);

				wxButton* addR = new wxButton(panel, wxID_ANY, "+", wxDefaultPosition, FromDIP(wxSize(40, -1)));
				gridSizer->Add(addR, 1, wxALIGN_CENTER);

				Data* currData = new Data(l, r, text, charcode, subL, subR, addL, addR);

				current = new Node(currData, current, next);

				if (first)
				{
					data[i->first] = current;
					first = false;
				}

				subL->Bind(wxEVT_BUTTON, &CssRedirectSelection::onSubL, this, wxID_ANY, wxID_ANY, new wxCSSArgument(i->first, current, gridSizer));
				addL->Bind(wxEVT_BUTTON, &CssRedirectSelection::onAddL, this, wxID_ANY, wxID_ANY, new wxCSSArgument(i->first, current, gridSizer));
				subR->Bind(wxEVT_BUTTON, &CssRedirectSelection::onSubR, this, wxID_ANY, wxID_ANY, new wxCSSArgument(i->first, current, gridSizer));
				addR->Bind(wxEVT_BUTTON, &CssRedirectSelection::onAddR, this, wxID_ANY, wxID_ANY, new wxCSSArgument(i->first, current, gridSizer));
			}
		}
		else
		{
			wxTextCtrl* charcode = new wxTextCtrl(panel, wxID_ANY, i->first, wxDefaultPosition, wxDefaultSize, wxTE_READONLY | wxTE_CENTRE);
			gridSizer->Add(charcode, 1, wxALIGN_CENTER | wxRIGHT, 20);

			wxButton* subL = new wxButton(panel, wxID_ANY, "-", wxDefaultPosition, FromDIP(wxSize(40, -1)));
			gridSizer->Add(subL, 1, wxALIGN_CENTER);

			wxButton* addL = new wxButton(panel, wxID_ANY, "+", wxDefaultPosition, FromDIP(wxSize(40, -1)));
			gridSizer->Add(addL, 1, wxALIGN_CENTER | wxRIGHT, 5);

			Slot l(0);
			Slot r(mHandler->getMaxSlot(i->first).getInt());

			wxStaticText* text = new wxStaticText(panel, wxID_ANY, "c" + l.getString() + " - c" + r.getString());
			gridSizer->Add(text, 1, wxALIGN_CENTER | wxRIGHT, 5);

			wxButton* subR = new wxButton(panel, wxID_ANY, "-", wxDefaultPosition, FromDIP(wxSize(40, -1)));
			gridSizer->Add(subR, 1, wxALIGN_CENTER);

			wxButton* addR = new wxButton(panel, wxID_ANY, "+", wxDefaultPosition, FromDIP(wxSize(40, -1)));
			gridSizer->Add(addR, 1, wxALIGN_CENTER);

			Data* currData = new Data(l, r, text, charcode, subL, subR, addL, addR);
			data[i->first] = new Node(currData);

			subL->Bind(wxEVT_BUTTON, &CssRedirectSelection::onSubL, this, wxID_ANY, wxID_ANY, new wxCSSArgument(i->first, data[i->first], gridSizer));
			addL->Bind(wxEVT_BUTTON, &CssRedirectSelection::onAddL, this, wxID_ANY, wxID_ANY, new wxCSSArgument(i->first, data[i->first], gridSizer));
			subR->Bind(wxEVT_BUTTON, &CssRedirectSelection::onSubR, this, wxID_ANY, wxID_ANY, new wxCSSArgument(i->first, data[i->first], gridSizer));
			addR->Bind(wxEVT_BUTTON, &CssRedirectSelection::onAddR, this, wxID_ANY, wxID_ANY, new wxCSSArgument(i->first, data[i->first], gridSizer));
		}

		sizer->Add(gridSizer, 1, wxALIGN_CENTER_HORIZONTAL | wxBOTTOM, FromDIP(20));
	}

	panel->SetSizerAndFit(sizer);

	wxBoxSizer* sizerV = new wxBoxSizer(wxVERTICAL);
	sizerV->Add(panel, 1, wxEXPAND | wxLEFT | wxTOP | wxRIGHT, FromDIP(20));
	sizerV->Add(CreateButtonSizer(wxOK | wxCANCEL), 0, wxALIGN_CENTER_HORIZONTAL | wxLEFT | wxRIGHT | wxBOTTOM, FromDIP(20));

	/*wxButton* okay = new wxButton(this, wxID_ANY, "Okay");
	okay->Bind(wxEVT_BUTTON, &CssRedirectSelection::onClosePressed, this, wxID_ANY, wxID_ANY, new wxArgument("okay"));

	wxButton* cancel = new wxButton(this, wxID_ANY, "Cancel");
	cancel->Bind(wxEVT_BUTTON, &CssRedirectSelection::onClosePressed, this, wxID_ANY, wxID_ANY, new wxArgument("cancel"));*/

	this->SetSizerAndFit(sizerV);
}

map<string, map<int, CssData>> CssRedirectSelection::getChanges()
{
	map<string, map<int, CssData>> result;

	for (auto i = data.begin(); i != data.end(); i++)
	{
		string ogCode = i->first;
		auto iter = i->second;

		while (iter)
		{
			result[ogCode][iter->data->left.getInt()].code = iter->data->charcode->GetValue().ToStdString();
			result[ogCode][iter->data->left.getInt()].original_ui_chara_hash = "ui_chara_" + iter->data->charcode->GetValue().ToStdString();
			result[ogCode][iter->data->left.getInt()].color_start_index = iter->data->left.getInt();
			result[ogCode][iter->data->left.getInt()].color_num = (iter->data->right.getInt() + 1) - iter->data->left.getInt();
			iter = iter->next;
		}
	}

	return result;
}

map<string, map<string, map<Slot, Slot>>> CssRedirectSelection::getRedirects()
{
	map<string, map<string, map<Slot, Slot>>> result;

	for (auto i = data.begin(); i != data.end(); i++)
	{
		auto iter = i->second;
		while (iter)
		{
			for (int j = 0; j < iter->data->right.getInt() + 1 - iter->data->left.getInt(); j++)
			{
				result[iter->data->charcode->GetValue().ToStdString()][i->first][Slot(j)] = iter->data->left.getInt() + j;
				if (result[iter->data->charcode->GetValue().ToStdString()][i->first][Slot(j)].getInt() == -1)
				{
					result[iter->data->charcode->GetValue().ToStdString()][i->first][Slot(j)] = Slot(00);
				}
			}
			iter = iter->next;
		}
	}

	return result;
}

Data::Data(Slot left, Slot right, wxStaticText* text, wxTextCtrl* charcode,
	wxButton* subL, wxButton* subR, wxButton* addL, wxButton* addR)
{
	this->left = left;
	this->right = right;
	this->text = text;
	this->charcode = charcode;

	this->subL = subL;
	this->subR = subR;
	this->addL = addL;
	this->addR = addR;
}

void Data::subLeft(int num)
{
	left.set(left.getInt() - num);
	text->SetLabel("c" + left.getString() + " - c" + right.getString());
}

void Data::subRight(int num)
{
	right.set(right.getInt() - num);
	text->SetLabel("c" + left.getString() + " - c" + right.getString());
}

void Data::addLeft(int num)
{
	left.set(left.getInt() + num);
	text->SetLabel("c" + left.getString() + " - c" + right.getString());
}
void Data::addRight(int num)
{
	right.set(right.getInt() + num);
	text->SetLabel("c" + left.getString() + " - c" + right.getString());
}

void Data::destroy()
{
	this->charcode->Destroy();
	this->text->Destroy();
	this->subL->Destroy();
	this->subR->Destroy();
	this->addL->Destroy();
	this->addR->Destroy();
}

Node::Node(Data* data, Node* prev, Node* next)
{
	this->data = data;
	this->prev = nullptr;
	this->next = nullptr;

	if (prev)
	{
		this->prev = prev;
		prev->next = this;
	}

	if (next)
	{
		this->next = next;
		next->prev = this;
	}
}

void Node::deleteMid(Node* prev, Node* current, Node* next)
{
	if (prev && next)
	{
		prev->next = next;
		next->prev = prev;
	}
	else if (prev)
	{
		prev->next = current ? current->next : nullptr;
	}
	else if (next)
	{
		next->prev = current ? current->prev : nullptr;
	}

	if (current)
	{
		if (current->data)
		{
			current->data->destroy();
		}
		delete current;
	}
}

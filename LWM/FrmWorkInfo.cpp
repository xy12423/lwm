#include "stdafx.h"
#include "FrmWorkInfo.h"

#ifdef __WXMSW__
#define _GUI_SIZE_X 300
#define _GUI_SIZE_Y 300
#else
#define _GUI_SIZE_X 280
#define _GUI_SIZE_Y 260
#endif

wxBEGIN_EVENT_TABLE(FrmWorkInfo, wxDialog)

EVT_BUTTON(ID_BUTTONAPPLY, FrmWorkInfo::buttonApply_Click)

wxEND_EVENT_TABLE()

FrmWorkInfo::FrmWorkInfo(const wxString &title, const wxString &_name, const wxString &_info)
	: wxDialog(NULL, ID_FRAME, title, wxDefaultPosition, wxSize(_GUI_SIZE_X, _GUI_SIZE_Y))
{
	Center();
	panel = new wxPanel(this);
	wxStaticText *label;

	label = new wxStaticText(panel, ID_ANY,
		wxT("工作名"),
		wxPoint(12, 12),
		wxSize(47, 21)
		);
	textName = new wxTextCtrl(panel, ID_TEXTNAME,
		_name,
		wxPoint(59, 12),
		wxSize(213, 21)
		);
	label = new wxStaticText(panel, ID_ANY,
		wxT("详细信息"),
		wxPoint(12, 39),
		wxSize(58, 21)
		);
	textInfo = new wxTextCtrl(panel, ID_TEXTINFO,
		_info,
		wxPoint(14, 60),
		wxSize(258, 157),
		wxTE_MULTILINE
		);
	buttonApply = new wxButton(panel, ID_BUTTONAPPLY,
		wxT("完成"),
		wxPoint(14, 223),
		wxSize(258, 30)
		);
}

void FrmWorkInfo::buttonApply_Click(wxCommandEvent& event)
{
	Close();
}

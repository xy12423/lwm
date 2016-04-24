#include "stdafx.h"
#include "FrmLogin.h"

#ifdef __WXMSW__
const int _GUI_SIZE_X = 300;
const int _GUI_SIZE_Y = 130;
#else
const int _GUI_SIZE_X = 280;
const int _GUI_SIZE_Y = 110;
#endif

FrmLogin::FrmLogin(const wxString &title)
	:wxDialog(NULL, ID_FRAME, title, wxDefaultPosition, wxSize(_GUI_SIZE_X, _GUI_SIZE_Y))
{
	Center();

	panel = new wxPanel(this);
	wxStaticText *label;

	label = new wxStaticText(panel, ID_ANY,
		wxT("用户名"),
		wxPoint(12, 12),
		wxSize(50, 21)
		);
	label = new wxStaticText(panel, ID_ANY,
		wxT("密码"),
		wxPoint(12, 39),
		wxSize(50, 21)
		);
	textName = new wxTextCtrl(panel, ID_TEXTNAME,
		wxEmptyString,
		wxPoint(68, 12),
		wxSize(204, 21)
		);
	textPass = new wxTextCtrl(panel, ID_TEXTPASS,
		wxEmptyString,
		wxPoint(68, 39),
		wxSize(204, 21),
		wxTE_PASSWORD
		);
	buttonOK = new wxButton(panel, wxID_OK,
		wxT("确认"),
		wxPoint(78, 66),
		wxSize(64, 26)
		);
	buttonCancel = new wxButton(panel, wxID_CANCEL,
		wxT("取消"),
		wxPoint(148, 66),
		wxSize(64, 26)
		);
}

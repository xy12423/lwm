#include "stdafx.h"
#include "crypto.h"
#include "session.h"
#include "main.h"
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
		wxT("�û���"),
		wxPoint(12, 12),
		wxSize(50, 21)
		);
	label = new wxStaticText(panel, ID_ANY,
		wxT("����"),
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
		wxT("ȷ��"),
		wxPoint(78, 66),
		wxSize(64, 26)
		);
	buttonCancel = new wxButton(panel, wxID_CANCEL,
		wxT("ȡ��"),
		wxPoint(148, 66),
		wxSize(64, 26)
		);
}

#include "stdafx.h"
#include "crypto.h"
#include "session.h"
#include "main.h"
#include "FrmLogin.h"

#ifdef __WXMSW__
const int _GUI_SIZE_X = 294;
const int _GUI_SIZE_Y = 111;
#else
const int _GUI_SIZE_X = 274;
const int _GUI_SIZE_Y = 91;
#endif

FrmLogin::FrmLogin(const wxString &title)
	:wxFrame(NULL, ID_FRAME, title, wxDefaultPosition, wxSize(_GUI_SIZE_X, _GUI_SIZE_Y))
{
	Center();

	panel = new wxPanel(this);
	wxStaticText *label;

	label = new wxStaticText(panel, wxID_ANY,
		wxT("”√ªß√˚"),
		wxPoint(12, 12),
		wxSize(50, 21)
		);
	label = new wxStaticText(panel, wxID_ANY,
		wxT("√‹¬Î"),
		wxPoint(12, 39),
		wxSize(50, 21)
		);
	textName = new wxTextCtrl(panel, ID_TEXTNAME,
		wxEmptyString,
		wxPoint(68, 12),
		wxSize(198, 21)
		);
	textPass = new wxTextCtrl(panel, ID_TEXTPASS,
		wxEmptyString,
		wxPoint(68, 39),
		wxSize(198, 21),
		wxTE_PASSWORD
		);
}

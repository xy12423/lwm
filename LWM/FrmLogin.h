#pragma once

#ifndef _H_FRM_LOGIN
#define _H_FRM_LOGIN

class FrmLogin : public wxDialog
{
public:
	FrmLogin(const wxString& title);

	wxString GetName() { return textName->GetValue(); }
	wxString GetPass() { return textPass->GetValue(); }
private:
	enum itemID {
		ID_FRAME = 100, ID_ANY,
		ID_TEXTNAME, ID_TEXTPASS
	};

	wxPanel *panel;

	wxTextCtrl *textName, *textPass;
	wxButton *buttonOK, *buttonCancel;
};

#endif

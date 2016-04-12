#pragma once

#ifndef _H_FRM_LOGIN

class FrmLogin : public wxFrame
{
public:
	FrmLogin(const wxString& title);

	wxString GetAddress() { return textName->GetValue(); }
	wxString GetPort() { return textPass->GetValue(); }
private:
	enum itemID {
		ID_FRAME = 100,
		ID_TEXTNAME, ID_TEXTPASS
	};

	wxPanel *panel;

	wxTextCtrl *textName, *textPass;
	wxButton *buttonOK, *buttonCancel;
};

#endif

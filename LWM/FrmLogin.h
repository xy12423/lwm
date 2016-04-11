#pragma once

#ifndef _H_FRM_LOGIN

class FrmLogin : public wxFrame
{
public:
	FrmLogin(const wxString& title);
private:
	enum itemID {
		ID_FRAME
	};

	wxPanel *panel;

	wxDECLARE_EVENT_TABLE();
};

#endif

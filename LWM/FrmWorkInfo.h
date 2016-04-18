#pragma once

#ifndef _H_FRM_WORKINFO
#define _H_FRM_WORKINFO

class FrmWorkInfo : public wxDialog
{
public:
	FrmWorkInfo(const wxString &title, const wxString &_name, const wxString &_info);

	wxString GetName() { return textName->GetValue(); }
	wxString GetInfo() { return textInfo->GetValue(); }
private:
	enum itemID {
		ID_FRAME = 200, ID_ANY,
		ID_TEXTNAME, ID_TEXTINFO, ID_BUTTONAPPLY
	};

	wxPanel *panel;

	wxTextCtrl *textName, *textInfo;
	wxButton *buttonApply;
	void buttonApply_Click(wxCommandEvent& event);

	wxDECLARE_EVENT_TABLE();
};

#endif

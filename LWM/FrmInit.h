#pragma once

#ifndef _H_FRM_INIT
#define _H_FRM_INIT

class FrmInit : public wxFrame
{
public:
	FrmInit(const wxString& title);

	void SetStage(int _stage);
	void NextStage() { SetStage(stage + 1); };
private:
	enum itemID {
		ID_FRAME = 300, ID_ANY,
		ID_LABEL
	};

	wxPanel *panel;

	wxStaticText *label;

	int stage;
};

#endif

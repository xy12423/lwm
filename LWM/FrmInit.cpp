#include "stdafx.h"
#include "FrmInit.h"

#ifdef __WXMSW__
const int _GUI_SIZE_X = 300;
const int _GUI_SIZE_Y = 130;
#else
const int _GUI_SIZE_X = 280;
const int _GUI_SIZE_Y = 110;
#endif

wxString StageText[] = {
	wxT("正在连接至服务器..."),
	wxT("登录中..."),
	wxT("接收组信息..."),
	wxT("接收工作信息..."),
	wxT("接收用户信息..."),
};

FrmInit::FrmInit(const wxString &title)
	:wxFrame(NULL, ID_FRAME, title, wxDefaultPosition, wxSize(_GUI_SIZE_X, _GUI_SIZE_Y))
{
	Center();

	panel = new wxPanel(this);

	label = new wxStaticText(panel, ID_LABEL,
		wxEmptyString,
		wxPoint(80, 35),
		wxSize(150, 21)
	);
}

void FrmInit::SetStage(int stage)
{
	assert(stage >= 0 && stage < 4);
	label->SetLabelText(StageText[stage]);
}

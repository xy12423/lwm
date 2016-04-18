#include "stdafx.h"
#include "crypto.h"
#include "session.h"
#include "lwm_client.h"
#include "main.h"
#include "FrmLogin.h"

wxBEGIN_EVENT_TABLE(mainFrame, wxFrame)

wxEND_EVENT_TABLE()

#ifdef __WXMSW__
const int _GUI_GAP = 20;
const int _GUI_SIZE_X = 800;
const int _GUI_SIZE_Y = 600;
#else
const int _GUI_GAP = 0;
const int _GUI_SIZE_X = 780;
const int _GUI_SIZE_Y = 580;
#endif

extern lwm_client client;

const size_t status_count = 3;
wxString status_list[status_count] = {
	wxT("����"),
	wxT("æ"),
	wxT("������")
};

mainFrame::mainFrame(const wxString &title)
	:wxFrame(NULL, ID_FRAME, title, wxDefaultPosition, wxSize(_GUI_SIZE_X, _GUI_SIZE_Y))
{
	Center();

	panel = new wxPanel(this);
	wxStaticText *label;
	wxStaticBox *groupBox;

	groupBox = new wxStaticBox(panel, ID_ANY,
		wxT("��"),
		wxPoint(12, 12),
		wxSize(232, 254)
		);

	wxArrayString EmptyList, StatusList(status_count, status_list);
	listGroup = new wxCheckListBox(groupBox, ID_LISTGROUP,
		wxPoint(6, _GUI_GAP),
		wxSize(220, 196),
		EmptyList
		);
	buttonGroupAdd = new wxButton(groupBox, ID_BUTTONGROUPADD,
		wxT(""),
		wxPoint(6, _GUI_GAP + 202),
		wxSize(69, 26)
		);
	buttonGroupRename = new wxButton(groupBox, ID_BUTTONGROUPRENAME,
		wxT(""),
		wxPoint(81, _GUI_GAP + 202),
		wxSize(70, 26)
		);
	buttonGroupDel = new wxButton(groupBox, ID_BUTTONGROUPDEL,
		wxT(""),
		wxPoint(157, _GUI_GAP + 202),
		wxSize(69, 26)
		);

	groupBox = new wxStaticBox(panel, ID_ANY,
		wxT("����"),
		wxPoint(12, 272),
		wxSize(232, 277)
		);

	listWork = new wxCheckListBox(groupBox, ID_LISTWORK,
		wxPoint(6, _GUI_GAP),
		wxSize(220, 212),
		EmptyList
		);
	buttonWorkAdd = new wxButton(groupBox, ID_BUTTONWORKADD,
		wxT(""),
		wxPoint(6, _GUI_GAP + 218),
		wxSize(50, 33)
		);
	buttonWorkInfo = new wxButton(groupBox, ID_BUTTONWORKINFO,
		wxT(""),
		wxPoint(62, _GUI_GAP + 218),
		wxSize(51, 33)
		);
	buttonWorkEdit = new wxButton(groupBox, ID_BUTTONWORKEDIT,
		wxT(""),
		wxPoint(119, _GUI_GAP + 218),
		wxSize(51, 33)
		);
	buttonWorkDel = new wxButton(groupBox, ID_BUTTONWORKDEL,
		wxT(""),
		wxPoint(176, _GUI_GAP + 218),
		wxSize(50, 33)
		);

	groupBox = new wxStaticBox(panel, ID_ANY,
		wxT("��Ա"),
		wxPoint(250, 12),
		wxSize(218, 537)
		);

	label = new wxStaticText(groupBox, ID_ANY,
		wxT("��״̬ɸѡ"),
		wxPoint(6, _GUI_GAP),
		wxSize(70, 21)
		);
	choiceStatusFilter = new wxChoice(groupBox, ID_CHOICESTATUSFILTER,
		wxPoint(82, _GUI_GAP - 2),
		wxSize(130, 24),
		StatusList
		);
	listMember = new wxListBox(groupBox, ID_LISTMEMBER,
		wxPoint(6, _GUI_GAP + 27),
		wxSize(206, 448),
		EmptyList
		);
	buttonMemberAdd = new wxButton(groupBox, ID_BUTTONMEMBERADD,
		wxT(""),
		wxPoint(6, _GUI_GAP + 481),
		wxSize(100, 30)
		);
	buttonMemberDel = new wxButton(groupBox, ID_BUTTONMEMBERDEL,
		wxT(""),
		wxPoint(112, _GUI_GAP + 481),
		wxSize(100, 30)
		);

	groupBox = new wxStaticBox(panel, ID_ANY,
		wxT("��Ա��Ϣ"),
		wxPoint(474, 12),
		wxSize(298, 537)
		);

	label = new wxStaticText(groupBox, ID_ANY,
		wxT("ID"),
		wxPoint(6, _GUI_GAP),
		wxSize(26, 21)
		);
	textName = new wxTextCtrl(groupBox, ID_TEXTNAME,
		wxT(""),
		wxPoint(38, _GUI_GAP),
		wxSize(189, 21)
		);
	choiceStatus = new wxChoice(groupBox, ID_CHOICESTATUS,
		wxPoint(233, _GUI_GAP - 2),
		wxSize(59, 24),
		StatusList
		);
	label = new wxStaticText(groupBox, ID_ANY,
		wxT("��Դ"),
		wxPoint(6, _GUI_GAP + 27),
		wxSize(36, 21)
		);
	textSource = new wxTextCtrl(groupBox, ID_TEXTSOURCE,
		wxT(""),
		wxPoint(48, _GUI_GAP + 27),
		wxSize(244, 21)
		);
	label = new wxStaticText(groupBox, ID_ANY,
		wxT("��ϸ��Ϣ"),
		wxPoint(6, _GUI_GAP + 54),
		wxSize(60, 21)
		);
	textInfo = new wxTextCtrl(groupBox, ID_TEXTINFO,
		wxT(""),
		wxPoint(6, _GUI_GAP + 81),
		wxSize(286, 153),
		wxTE_MULTILINE
		);
	buttonMemberApply = new wxButton(groupBox, ID_BUTTONMEMBERAPPLY,
		wxT(""),
		wxPoint(6, _GUI_GAP + 240),
		wxSize(286, 37)
		);
	listMemberGroup = new wxCheckListBox(groupBox, ID_LISTMEMBERGROUP,
		wxPoint(6, _GUI_GAP + 283),
		wxSize(140, 228),
		EmptyList
		);
	listMemberWork = new wxCheckListBox(groupBox, ID_LISTMEMBERWORK,
		wxPoint(152, _GUI_GAP + 283),
		wxSize(140, 228),
		EmptyList
		);

	FrmLogin login(wxT("��¼"));
	login.ShowModal();

	std::string name(wxConvUTF8.cWC2MB(login.GetName().c_str()));
	std::string pass(wxConvUTF8.cWC2MB(login.GetPass().c_str()));

	std::promise<int> login_promise;
	std::future<int> login_future = login_promise.get_future();
	client.set_callback([&login_promise](const lwm_client::response &response) {
		login_promise.set_value(response.err);
	});
	client.login(name, pass);
	if (login_future.get() != lwm_client::ERR_SUCCESS)
	{
		wxMessageBox(wxT("��¼ʧ��"), "Error", wxOK | wxICON_ERROR);
	}
}

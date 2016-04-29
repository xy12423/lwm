#include "stdafx.h"
#include "structure.h"
#include "FrmMain.h"

wxBEGIN_EVENT_TABLE(FrmMain, wxFrame)

EVT_CHECKLISTBOX(ID_LISTGROUP, listGroup_ItemCheck)
EVT_BUTTON(ID_BUTTONGROUPADD, buttonGroupAdd_Click)
EVT_BUTTON(ID_BUTTONGROUPRENAME, buttonGroupRename_Click)
EVT_BUTTON(ID_BUTTONGROUPDEL, buttonGroupDel_Click)

EVT_CHECKLISTBOX(ID_LISTWORK, listWork_ItemCheck)
EVT_BUTTON(ID_BUTTONWORKADD, buttonWorkAdd_Click)
EVT_BUTTON(ID_BUTTONWORKEDIT, buttonWorkEdit_Click)
EVT_BUTTON(ID_BUTTONWORKINFO, buttonWorkInfo_Click)
EVT_BUTTON(ID_BUTTONWORKDEL, buttonWorkDel_Click)

EVT_LISTBOX(ID_LISTMEMBER, listMember_SelectedIndexChanged)
EVT_BUTTON(ID_BUTTONMEMBERADD, buttonMemberAdd_Click)
EVT_BUTTON(ID_BUTTONMEMBERDEL, buttonMemberDel_Click)

EVT_BUTTON(ID_BUTTONMEMBERAPPLY, buttonMemberApply_Click)
EVT_CHECKLISTBOX(ID_LISTMEMBERGROUP, listMemberGroup_ItemCheck)
EVT_CHECKLISTBOX(ID_LISTMEMBERWORK, listMemberWork_ItemCheck)

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

const size_t status_count = 3;
wxString status_list[status_count] = {
	wxT("可用"),
	wxT("忙"),
	wxT("不可用")
};

void FrmMain::RefreshMemberList()
{
	wxArrayInt checkedG, checkedW;
	listGroup->GetCheckedItems(checkedG);
	listWork->GetCheckedItems(checkedW);
	std::unordered_set<id_type_l> checkedGID, checkedWID;
	for (auto itr = checkedG.begin(), itrEnd = checkedG.end(); itr != itrEnd; itr++)
		checkedGID.insert(GIDMap[*itr]);
	for (auto itr = checkedW.begin(), itrEnd = checkedW.end(); itr != itrEnd; itr++)
		checkedWID.insert(WIDMap[*itr]);

	listMember->Clear();
	UIDMap.clear();
	for (auto itr = memList.cbegin(), itrEnd = memList.cend(); itr != itrEnd; itr++)
	{
		const member &mem = itr->second;
		bool flag = false;
		for (id_type_l gID : checkedGID)
		{
			if (mem.isInGroup(gID))
			{
				flag = true;
				break;
			}
		}
		if (!flag)
			continue;
		for (id_type_l wID : checkedWID)
		{
			if (mem.isInWork(wID))
			{
				listMember->Append(mem.getName());
				UIDMap.push_back(mem.getUID());
				break;
			}
		}
	}
}

FrmMain::FrmMain(const wxString &title)
	:wxFrame(NULL, ID_FRAME, title, wxDefaultPosition, wxSize(_GUI_SIZE_X, _GUI_SIZE_Y))
{
	Center();

	panel = new wxPanel(this);
	wxStaticText *label;
	wxStaticBox *groupBox;

	groupBox = new wxStaticBox(panel, ID_ANY,
		wxT("组"),
		wxPoint(12, 12),
		wxSize(232, 254)
		);

	wxArrayString EmptyList, GroupList, WorkList, StatusList(status_count, status_list);

	for (const std::pair<id_type, group> &p : grpList)
	{
		GIDMap.push_back(p.second.getGID());
		GroupList.push_back(p.second.getName());
	}
	for (const std::pair<id_type, work> &p : workList)
	{
		WIDMap.push_back(p.second.getWID());
		WorkList.push_back(p.second.getName());
	}

	listGroup = new wxCheckListBox(groupBox, ID_LISTGROUP,
		wxPoint(6, _GUI_GAP),
		wxSize(220, 196),
		GroupList
		);
	buttonGroupAdd = new wxButton(groupBox, ID_BUTTONGROUPADD,
		wxT("添加"),
		wxPoint(6, _GUI_GAP + 202),
		wxSize(69, 26)
		);
	buttonGroupRename = new wxButton(groupBox, ID_BUTTONGROUPRENAME,
		wxT("重命名"),
		wxPoint(81, _GUI_GAP + 202),
		wxSize(70, 26)
		);
	buttonGroupDel = new wxButton(groupBox, ID_BUTTONGROUPDEL,
		wxT("删除"),
		wxPoint(157, _GUI_GAP + 202),
		wxSize(69, 26)
		);

	groupBox = new wxStaticBox(panel, ID_ANY,
		wxT("工作"),
		wxPoint(12, 272),
		wxSize(232, 277)
		);

	listWork = new wxCheckListBox(groupBox, ID_LISTWORK,
		wxPoint(6, _GUI_GAP),
		wxSize(220, 212),
		WorkList
		);
	buttonWorkAdd = new wxButton(groupBox, ID_BUTTONWORKADD,
		wxT("添加"),
		wxPoint(6, _GUI_GAP + 218),
		wxSize(50, 33)
		);
	buttonWorkInfo = new wxButton(groupBox, ID_BUTTONWORKINFO,
		wxT("信息"),
		wxPoint(62, _GUI_GAP + 218),
		wxSize(51, 33)
		);
	buttonWorkEdit = new wxButton(groupBox, ID_BUTTONWORKEDIT,
		wxT("更改"),
		wxPoint(119, _GUI_GAP + 218),
		wxSize(51, 33)
		);
	buttonWorkDel = new wxButton(groupBox, ID_BUTTONWORKDEL,
		wxT("删除"),
		wxPoint(176, _GUI_GAP + 218),
		wxSize(50, 33)
		);

	groupBox = new wxStaticBox(panel, ID_ANY,
		wxT("人员"),
		wxPoint(250, 12),
		wxSize(218, 537)
		);

	label = new wxStaticText(groupBox, ID_ANY,
		wxT("按状态筛选"),
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
		wxT("添加"),
		wxPoint(6, _GUI_GAP + 481),
		wxSize(100, 30)
		);
	buttonMemberDel = new wxButton(groupBox, ID_BUTTONMEMBERDEL,
		wxT("删除"),
		wxPoint(112, _GUI_GAP + 481),
		wxSize(100, 30)
		);

	groupBox = new wxStaticBox(panel, ID_ANY,
		wxT("人员信息"),
		wxPoint(474, 12),
		wxSize(298, 537)
		);

	label = new wxStaticText(groupBox, ID_ANY,
		wxT("ID"),
		wxPoint(6, _GUI_GAP),
		wxSize(26, 21)
		);
	textName = new wxTextCtrl(groupBox, ID_TEXTNAME,
		wxEmptyString,
		wxPoint(38, _GUI_GAP),
		wxSize(189, 21)
		);
	choiceStatus = new wxChoice(groupBox, ID_CHOICESTATUS,
		wxPoint(233, _GUI_GAP - 2),
		wxSize(59, 24),
		StatusList
		);
	label = new wxStaticText(groupBox, ID_ANY,
		wxT("来源"),
		wxPoint(6, _GUI_GAP + 27),
		wxSize(36, 21)
		);
	textSource = new wxTextCtrl(groupBox, ID_TEXTSOURCE,
		wxEmptyString,
		wxPoint(48, _GUI_GAP + 27),
		wxSize(244, 21)
		);
	label = new wxStaticText(groupBox, ID_ANY,
		wxT("详细信息"),
		wxPoint(6, _GUI_GAP + 54),
		wxSize(60, 21)
		);
	textInfo = new wxTextCtrl(groupBox, ID_TEXTINFO,
		wxEmptyString,
		wxPoint(6, _GUI_GAP + 81),
		wxSize(286, 153),
		wxTE_MULTILINE
		);
	buttonMemberApply = new wxButton(groupBox, ID_BUTTONMEMBERAPPLY,
		wxT("应用更改"),
		wxPoint(6, _GUI_GAP + 240),
		wxSize(286, 37)
		);
	listMemberGroup = new wxCheckListBox(groupBox, ID_LISTMEMBERGROUP,
		wxPoint(6, _GUI_GAP + 283),
		wxSize(140, 228),
		GroupList
		);
	listMemberWork = new wxCheckListBox(groupBox, ID_LISTMEMBERWORK,
		wxPoint(152, _GUI_GAP + 283),
		wxSize(140, 228),
		WorkList
		);

	for (unsigned int i = 0; i < GroupList.size(); i++)
		listGroup->Check(i);
	for (unsigned int i = 0; i < WorkList.size(); i++)
		listWork->Check(i);
	RefreshMemberList();
}

void FrmMain::listGroup_ItemCheck(wxCommandEvent& event)
{
	RefreshMemberList();
}

void FrmMain::buttonGroupAdd_Click(wxCommandEvent& event)
{

}

void FrmMain::buttonGroupRename_Click(wxCommandEvent& event)
{

}

void FrmMain::buttonGroupDel_Click(wxCommandEvent& event)
{

}

void FrmMain::listWork_ItemCheck(wxCommandEvent& event)
{
	RefreshMemberList();
}

void FrmMain::buttonWorkAdd_Click(wxCommandEvent& event)
{

}

void FrmMain::buttonWorkEdit_Click(wxCommandEvent& event)
{

}

void FrmMain::buttonWorkInfo_Click(wxCommandEvent& event)
{

}

void FrmMain::buttonWorkDel_Click(wxCommandEvent& event)
{

}

void FrmMain::listMember_SelectedIndexChanged(wxCommandEvent& event)
{
	int selection = listMember->GetSelection();
	if (selection != -1)
	{
		const member &mem = memList.at(UIDMap[selection]);
		const uExtInfo &ext = mem.getExtInfo();
		textName->SetValue(mem.getName());
		textSource->SetValue(ext.src);
		textInfo->SetValue(ext.info);

		for (unsigned int i = 0; i < GIDMap.size(); i++)
		{
			if (mem.isInGroup(GIDMap[i]))
				listMemberGroup->Check(i, true);
			else
				listMemberGroup->Check(i, false);
		}
		for (unsigned int i = 0; i < WIDMap.size(); i++)
		{
			if (mem.isInWork(WIDMap[i]))
				listMemberWork->Check(i, true);
			else
				listMemberWork->Check(i, false);
		}
	}
}

void FrmMain::buttonMemberAdd_Click(wxCommandEvent& event)
{

}

void FrmMain::buttonMemberDel_Click(wxCommandEvent& event)
{

}

void FrmMain::buttonMemberApply_Click(wxCommandEvent& event)
{

}

void FrmMain::listMemberGroup_ItemCheck(wxCommandEvent& event)
{

}

void FrmMain::listMemberWork_ItemCheck(wxCommandEvent& event)
{

}

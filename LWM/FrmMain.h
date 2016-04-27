#pragma once

#ifndef _H_FRM_MAIN
#define _H_FRM_MAIN

class FrmMain : public wxFrame
{
public:
	FrmMain(const wxString &title);

	void RefreshGroup(id_type id);
	void RefreshWork(id_type id);
	void RefreshMember(id_type id);
private:
	enum itemID {
		ID_FRAME, ID_ANY,
		ID_LISTGROUP,
		ID_BUTTONGROUPADD, ID_BUTTONGROUPRENAME, ID_BUTTONGROUPDEL,
		ID_LISTWORK,
		ID_BUTTONWORKADD, ID_BUTTONWORKEDIT, ID_BUTTONWORKINFO, ID_BUTTONWORKDEL,
		ID_CHOICESTATUSFILTER,
		ID_LISTMEMBER,
		ID_BUTTONMEMBERADD, ID_BUTTONMEMBERDEL,
		ID_TEXTNAME, ID_TEXTSOURCE, ID_TEXTINFO,
		ID_CHOICESTATUS,
		ID_BUTTONMEMBERAPPLY,
		ID_LISTMEMBERGROUP, ID_LISTMEMBERWORK,
	};

	wxPanel *panel;

	wxCheckListBox *listGroup;
	wxButton *buttonGroupAdd, *buttonGroupRename, *buttonGroupDel;
	void listGroup_ItemCheck(wxCommandEvent& event);
	void buttonGroupAdd_Click(wxCommandEvent& event);
	void buttonGroupRename_Click(wxCommandEvent& event);
	void buttonGroupDel_Click(wxCommandEvent& event);

	wxCheckListBox *listWork;
	wxButton *buttonWorkAdd, *buttonWorkEdit, *buttonWorkInfo, *buttonWorkDel;
	void listWork_ItemCheck(wxCommandEvent& event);
	void buttonWorkAdd_Click(wxCommandEvent& event);
	void buttonWorkEdit_Click(wxCommandEvent& event);
	void buttonWorkInfo_Click(wxCommandEvent& event);
	void buttonWorkDel_Click(wxCommandEvent& event);

	wxChoice *choiceStatusFilter;
	wxListBox *listMember;
	wxButton *buttonMemberAdd, *buttonMemberDel;
	void listMember_SelectedIndexChanged(wxCommandEvent& event);
	void buttonMemberAdd_Click(wxCommandEvent& event);
	void buttonMemberDel_Click(wxCommandEvent& event);

	wxTextCtrl *textName, *textSource, *textInfo;
	wxChoice *choiceStatus;
	wxButton *buttonMemberApply;
	wxCheckListBox *listMemberGroup, *listMemberWork;
	void buttonMemberApply_Click(wxCommandEvent& event);
	void listMemberGroup_ItemCheck(wxCommandEvent& event);
	void listMemberWork_ItemCheck(wxCommandEvent& event);

	std::vector<id_type> GIDMap, WIDMap, UIDMap;

	wxDECLARE_EVENT_TABLE();
};

#endif

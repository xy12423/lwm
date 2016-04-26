#pragma once

#ifndef _H_FRM_MAIN
#define _H_FRM_MAIN

class FrmMain : public wxFrame
{
public:
	FrmMain(const wxString &title);

	friend class lwm_client;
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

	wxCheckListBox *listWork;
	wxButton *buttonWorkAdd, *buttonWorkEdit, *buttonWorkInfo, *buttonWorkDel;

	wxChoice *choiceStatusFilter;
	wxListBox *listMember;
	wxButton *buttonMemberAdd, *buttonMemberDel;

	wxTextCtrl *textName, *textSource, *textInfo;
	wxChoice *choiceStatus;
	wxButton *buttonMemberApply;
	wxCheckListBox *listMemberGroup, *listMemberWork;

	wxDECLARE_EVENT_TABLE();
};

#endif

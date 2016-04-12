#pragma once

#ifndef _H_MAIN
#define _H_MAIN

class mainFrame : public wxFrame
{
public:
	mainFrame(const wxString& title);
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

class lwm_client :public msgr_inter
{
public:
	virtual void on_data(user_id_type id, const std::string& data);

	virtual void on_join(user_id_type id, const std::string& key);
	virtual void on_leave(user_id_type id);

	virtual bool new_rand_port(port_type &port);
	virtual void free_rand_port(port_type port) { ports.push_back(port); };

	void set_frame(mainFrame *_frm) { frm = _frm; }
private:
	std::list<port_type> ports;

	mainFrame *frm;
};

class MyApp : public wxApp
{
public:
	virtual bool OnInit();
	virtual int OnExit();
private:
	mainFrame *form;
};

#endif

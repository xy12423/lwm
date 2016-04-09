#pragma once

#ifndef _H_MAIN
#define _H_MAIN

class mainFrame : public wxFrame
{
public:
	mainFrame(const wxString& title);
private:
	enum itemID {
		ID_FRAME,
		ID_LISTUSER, ID_BUTTONADD, ID_BUTTONDEL,
		ID_TEXTMSG, ID_TEXTINPUT, ID_BUTTONSEND, ID_BUTTONSENDIMAGE, ID_BUTTONSENDFILE, ID_BUTTONCANCELSEND,
		ID_TEXTINFO
	};

	wxPanel *panel;

	wxDECLARE_EVENT_TABLE();
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

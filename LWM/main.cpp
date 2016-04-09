#include "stdafx.h"
#include "main.h"

wxBEGIN_EVENT_TABLE(mainFrame, wxFrame)

wxEND_EVENT_TABLE()

#ifdef __WXMSW__
const int _GUI_SIZE_X = 620;
const int _GUI_SIZE_Y = 560;
#else
const int _GUI_SIZE_X = 600;
const int _GUI_SIZE_Y = 540;
#endif

mainFrame::mainFrame(const wxString &title)
	:wxFrame(NULL, ID_FRAME, title, wxDefaultPosition, wxSize(_GUI_SIZE_X, _GUI_SIZE_Y))
{
	Center();

	panel = new wxPanel(this);
	wxStaticText *label;
}

IMPLEMENT_APP(MyApp)

bool MyApp::OnInit()
{
	int stage = 0;
	try
	{
	}
	catch (std::exception &ex)
	{
		wxMessageBox(ex.what(), wxT("Error"), wxOK | wxICON_ERROR);
		return false;
	}

	return true;
}

int MyApp::OnExit()
{
	try
	{
	}
	catch (...)
	{
		return 1;
	}

	return 0;
}

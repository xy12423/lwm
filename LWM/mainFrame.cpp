#include "stdafx.h"
#include "crypto.h"
#include "session.h"
#include "main.h"

wxBEGIN_EVENT_TABLE(mainFrame, wxFrame)

wxEND_EVENT_TABLE()

#ifdef __WXMSW__
const int _GUI_GAP = 20;
const int _GUI_SIZE_X = 620;
const int _GUI_SIZE_Y = 560;
#else
const int _GUI_GAP = 0;
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

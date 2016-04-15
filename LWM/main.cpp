#include "stdafx.h"
#include "crypto.h"
#include "session.h"
#include "iosrvThread.h"
#include "lwm_client.h"
#include "main.h"

const char* privatekeyFile = ".privatekey";
const port_type portListenDefault = 4826;
port_type portListen = portListenDefault;

iosrvThread *threadNetwork, *threadMisc;

asio::io_service main_io_service, misc_io_service;
std::unique_ptr<msgr_proto::server> srv;
lwm_client client;

IMPLEMENT_APP(MyApp)

bool MyApp::OnInit()
{
	int stage = 0;
	try
	{
		initKey();

		port_type portsBegin = 5000, portsEnd = 9999;
		bool use_v6 = false;

		threadNetwork = new iosrvThread(main_io_service);
		stage = 1;
		threadMisc = new iosrvThread(misc_io_service);
		stage = 2;

		std::srand(static_cast<unsigned int>(std::time(NULL)));
		for (; portsBegin <= portsEnd; portsBegin++)
			client.free_rand_port(portsBegin);
		srv = std::make_unique<msgr_proto::server>(main_io_service, misc_io_service, client);

		form = new mainFrame(wxT("LWM"));
		form->Show();

		if (threadNetwork->Run() != wxTHREAD_NO_ERROR)
		{
			delete threadNetwork;
			throw(std::runtime_error("Can't run iosrvThread"));
		}
		if (threadMisc->Run() != wxTHREAD_NO_ERROR)
		{
			delete threadMisc;
			throw(std::runtime_error("Can't run iosrvThread"));
		}
	}
	catch (std::exception &ex)
	{
		switch (stage)
		{
			case 2:
				threadMisc->Delete();
			case 1:
				threadNetwork->Delete();
			default:
				break;
		}

		wxMessageBox(ex.what(), wxT("Error"), wxOK | wxICON_ERROR);
		return false;
	}

	return true;
}

int MyApp::OnExit()
{
	try
	{
		threadMisc->Delete();
		threadNetwork->Delete();

		srv.reset();
	}
	catch (...)
	{
		return 1;
	}

	return 0;
}

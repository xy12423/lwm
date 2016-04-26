#include "stdafx.h"
#include "session.h"
#include "iosrvThread.h"
#include "lwm_client.h"
#include "structure.h"
#include "main.h"
#include "FrmLogin.h"
#include "FrmInit.h"

const char* privatekeyFile = ".privatekey";

iosrvThread *threadNetwork, *threadMisc;

asio::io_service main_io_service, misc_io_service;
std::unique_ptr<msgr_proto::server> srv;
lwm_client client;

bool LWM::ConnectTo(const std::string &addr, port_type port)
{
	std::shared_ptr<std::promise<int>> connect_promise = std::make_shared<std::promise<int>>();
	std::future<int> connect_future = connect_promise->get_future();
	client.set_callback([connect_promise](const lwm_client::response &response) {
		try
		{
			connect_promise->set_value(response.err);
		}
		catch (...) {}
	});
	std::thread sleep_thread([connect_promise]() {
		wxSleep(10);
		try
		{
			connect_promise->set_value(lwm_client::ERR_TIMED_OUT);
		}
		catch (...) {}
	});
	sleep_thread.detach();
	client.connect(addr, port);

	return connect_future.get() == lwm_client::ERR_SUCCESS;
}

bool LWM::Login()
{
	FrmLogin login(wxT("登录"));
	login.ShowModal();

	std::string name(wxConvUTF8.cWC2MB(login.GetName().c_str()));
	std::string pass(wxConvUTF8.cWC2MB(login.GetPass().c_str()));

	std::promise<int> login_promise;
	std::future<int> login_future = login_promise.get_future();
	client.set_callback([&login_promise](lwm_client::response response) {
		login_promise.set_value(response.err);
	});
	client.login(name, pass);
	return login_future.get() == lwm_client::ERR_SUCCESS;
}

void LWM::OnResponse(lwm_client::response res)
{
	switch (res.req.op)
	{
		case lwm_client::OP_INFO:
			break;
	}
}

IMPLEMENT_APP(LWM)

bool LWM::OnInit()
{
	int stage = 0;
	try
	{
		initKey();

		std::ifstream fin(".config");
		if (!fin || !fin.is_open())
			throw(std::runtime_error("Config file not found"));

		std::string addr;
		port_type port;
		fin >> addr >> port;
		fin.close();

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

		if (threadNetwork->Run() != wxTHREAD_NO_ERROR)
			throw(std::runtime_error("Can't run iosrvThread"));
		if (threadMisc->Run() != wxTHREAD_NO_ERROR)
			throw(std::runtime_error("Can't run iosrvThread"));

		FrmInit init(wxT("初始化"));
		init.Show();
		init.SetStage(0);

		if (!ConnectTo(addr, port))
		{
			wxMessageBox(wxT("无法连接至服务器"), "Error", wxOK | wxICON_ERROR);
			throw(1);
		}
		init.SetStage(1);

		if (!Login())
		{
			wxMessageBox(wxT("登录失败"), "Error", wxOK | wxICON_ERROR);
			throw(1);
		}
		init.SetStage(2);

		if (list_group() != lwm_client::ERR_SUCCESS)
		{
			wxMessageBox(wxT("无法加载组信息"), "Error", wxOK | wxICON_ERROR);
			throw(1);
		}

		form = new FrmMain(wxT("LWM"));
		form->Show();
	}
	catch (int)
	{
		switch (stage)
		{
			case 2:
				threadMisc->Delete();
			case 1:
				threadNetwork->Delete();
			default:
				return false;
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

int LWM::OnExit()
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

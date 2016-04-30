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

std::function<void(lwm_client::response, std::shared_ptr<lwm_client::lwm_callback>)> FirstCallback;
void set_callback(lwm_client::lwm_callback &&callback)
{
	std::shared_ptr<lwm_client::lwm_callback> _callback = std::make_shared<lwm_client::lwm_callback>(callback);
	client.set_callback([_callback](lwm_client::response res) {
		FirstCallback(res, _callback);
		client.set_callback([](lwm_client::response res) {
			FirstCallback(res, std::make_shared<lwm_client::lwm_callback>());
		});
	});
}

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
	if (login.ShowModal() != wxID_OK)
		return false;

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

void LWM::OnResponse(lwm_client::response res, std::shared_ptr<lwm_client::lwm_callback> callback)
{
	if (callback)
	{
		try
		{
			(*callback)(res);
		}
		catch (...) {}
	}
	if (res.err == lwm_client::ERR_DISCONNECTED)
	{
		wxMessageBox(wxT("与服务器的连接已断开"), "Error", wxOK | wxICON_ERROR);
		form->Close();
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
		init.NextStage();

		if (!Login())
		{
			wxMessageBox(wxT("登录失败"), "Error", wxOK | wxICON_ERROR);
			throw(1);
		}
		FirstCallback = [this](lwm_client::response res, std::shared_ptr<lwm_client::lwm_callback> callback) { OnResponse(res, callback); };
		init.NextStage();

		if (list(lwm_client::CAT_GROUP) != lwm_client::ERR_SUCCESS)
		{
			wxMessageBox(wxT("无法加载组信息"), "Error", wxOK | wxICON_ERROR);
			throw(1);
		}
		init.NextStage();
		if (list(lwm_client::CAT_WORK) != lwm_client::ERR_SUCCESS)
		{
			wxMessageBox(wxT("无法加载工作信息"), "Error", wxOK | wxICON_ERROR);
			throw(1);
		}
		init.NextStage();
		if (list(lwm_client::CAT_MEMBER) != lwm_client::ERR_SUCCESS)
		{
			wxMessageBox(wxT("无法加载成员信息"), "Error", wxOK | wxICON_ERROR);
			throw(1);
		}
		init.NextStage();

		group &default_grp = grpList.emplace(default_id, group(default_id, wxT("[无组]"))).first->second;
		work &default_wrk = workList.emplace(default_id, work(default_id, wxT("[无工作]"), wxT(""))).first->second;
		for (memListTp::iterator itr = memList.begin(), itrEnd = memList.end(); itr != itrEnd; itr++)
		{
			member &mem = itr->second;
			if (mem.getGroupCount() == 0)
			{
				default_grp.addMember(mem.getUID());
				mem.addGroup(default_id);
			}
			if (mem.getWorkCount() == 0)
			{
				default_wrk.addMember(mem.getUID());
				mem.addWork(default_id);
			}
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

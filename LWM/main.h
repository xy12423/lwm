#pragma once

#ifndef _H_MAIN
#define _H_MAIN

#include "lwm_client.h"
#include "FrmMain.h"

class LWM : public wxApp
{
public:
	virtual bool OnInit();
	virtual int OnExit();
private:
	bool ConnectTo(const std::string &addr, port_type port);
	bool Login();

	void OnResponse(lwm_client::response res);

	FrmMain *form;
};

#endif

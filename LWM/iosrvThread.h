#pragma once

#ifndef _H_IOSRV_THREAD
#define _H_IOSRV_THREAD

class iosrvThread :public wxThread
{
public:
	iosrvThread(asio::io_service& _iosrv) : wxThread(wxTHREAD_DETACHED), iosrv(_iosrv) {};
	wxThreadError Delete(ExitCode *rc = NULL, wxThreadWait waitMode = wxTHREAD_WAIT_DEFAULT) { stop(); return wxThread::Delete(); };

	void stop() { iosrv_work.reset(); iosrv.stop(); }
protected:
	asio::io_service& iosrv;
	std::shared_ptr<asio::io_service::work> iosrv_work;

	ExitCode Entry();
};

#endif

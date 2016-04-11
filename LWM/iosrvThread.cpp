#include "stdafx.h"
#include "iosrvThread.h"

iosrvThread::ExitCode iosrvThread::Entry()
{
	iosrv_work = std::make_shared<asio::io_service::work>(iosrv);
	while (!TestDestroy())
	{
		try
		{
			iosrv.run();
		}
		catch (std::exception &ex) { std::cerr << ex.what() << std::endl; }
		catch (...) {}
	}
	return NULL;
}

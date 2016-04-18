#include "stdafx.h"
#include "crypto.h"
#include "session.h"
#include "main.h"

const std::string empty_string;
const char *privatekeyFile = ".privatekey";

std::promise<void> exit_promise;

const char OPERATION_SUCCESS = 0;
const char OPERATION_FAILURE = -1;

asio::io_service main_io_service, misc_io_service;
lwm_server inter;
config_table_tp config_items;
volatile bool server_on = true;

#define checkErr(x) if (dataItr + (x) > dataEnd) throw(0)
#define read_uint(x)												\
	checkErr(size_length);											\
	memcpy(reinterpret_cast<char*>(&(x)), dataItr, size_length);	\
	dataItr += size_length

void lwm_server::on_data(user_id_type id, const std::string &data)
{
	try
	{
		user_ext &user = user_exts.at(id);

		const size_t size_length = sizeof(data_length_type);
		const char *dataItr = data.data(), *dataEnd = data.data() + data.size();

		switch (user.current_stage)
		{
			case user_ext::LOGGING_IN:
			{
				data_length_type name_size, pass_size;
				
				read_uint(name_size);
				checkErr(name_size);
				user.name.assign(dataItr, name_size);
				dataItr += name_size;

				read_uint(pass_size);
				checkErr(pass_size);
				std::string pass(dataItr, pass_size);
				dataItr += pass_size;

				user_record_list::iterator itr = user_records.find(user.name);
				if (itr != user_records.end())
				{
					user_record &record = itr->second;
					std::string hashed_pass;
					hash(pass, hashed_pass);
					if (record.passwd == hashed_pass)
					{
						user.current_stage = user_ext::LOGGED_IN;
						record.logged_in = true;
						record.id = id;

						send_data(id, { OPERATION_SUCCESS }, msgr_proto::session::priority_sys);
					}
				}

				if (user.current_stage != user_ext::LOGGED_IN)
					send_data(id, { OPERATION_FAILURE }, msgr_proto::session::priority_sys);

				break;
			}
			case user_ext::LOGGED_IN:
			{
				break;
			}
		}
	}
	catch (std::exception &ex)
	{
		std::cerr << ex.what() << std::endl;
	}
	catch (int)
	{
	}
	catch (...)
	{
		throw;
	}
}

#undef read_uint
#undef checkErr

void lwm_server::on_join(user_id_type id, const std::string &)
{
	user_ext &ext = user_exts.emplace(id, user_ext()).first->second;
}

void lwm_server::on_leave(user_id_type id)
{
	user_ext_list::iterator itr = user_exts.find(id);
	user_ext &user = itr->second;

	user_exts.erase(itr);
}

void lwm_server::broadcast_data(int src, const std::string &data, int priority)
{
	for (const std::pair<int, user_ext> &p : user_exts)
	{
		int target = p.first;
		if (target != src && (p.second.current_stage == user_ext::LOGGED_IN))
		{
			send_data(static_cast<user_id_type>(target), data, priority);
		}
	}
}

bool lwm_server::new_rand_port(port_type &ret)
{
	if (static_port != -1)
		ret = static_cast<port_type>(static_port);
	else
	{
		if (ports.empty())
			return false;
		std::list<port_type>::iterator portItr = ports.begin();
		for (int i = std::rand() % ports.size(); i > 0; i--)
			portItr++;
		ret = *portItr;
		ports.erase(portItr);
	}
	return true;
}

int main(int argc, char *argv[])
{

#ifdef NDEBUG
	try
	{
#endif
		for (int i = 1; i < argc; i++)
		{
			std::string arg(argv[i]);
			size_t pos = arg.find('=');
			if (pos == std::string::npos)
				config_items[std::move(arg)] = empty_string;
			else
				config_items[arg.substr(0, pos)] = arg.substr(pos + 1);
		}

		port_type portListener = 4836;
		port_type portsBegin = 5000, portsEnd = 9999;
		bool use_v6 = false;

		try
		{
			std::string &arg = config_items.at("port");
			portListener = static_cast<port_type>(std::stoi(arg));
			std::cout << "Listening " << arg << std::endl;
		}
		catch (std::out_of_range &) { portListener = 4836; }
		catch (std::invalid_argument &) { portListener = 4836; }
		try
		{
			std::string &arg = config_items.at("ports");
			size_t pos = arg.find('-');
			if (pos == std::string::npos)
			{
				inter.set_static_port(static_cast<port_type>(std::stoi(arg)));
				portsBegin = 1;
				portsEnd = 0;
				std::cout << "Connecting port set to " << arg << std::endl;
			}
			else
			{
				std::string ports_begin = arg.substr(0, pos), ports_end = arg.substr(pos + 1);
				portsBegin = static_cast<port_type>(std::stoi(ports_begin));
				portsEnd = static_cast<port_type>(std::stoi(ports_end));
				inter.set_static_port(-1);
				std::cout << "Connecting ports set to " << arg << std::endl;
			}
		}
		catch (std::out_of_range &) { portsBegin = 5000, portsEnd = 9999; }
		catch (std::invalid_argument &) { portsBegin = 5000, portsEnd = 9999; }
		try
		{
			config_items.at("usev6");
			use_v6 = true;
			std::cout << "Using IPv6 for listening" << std::endl;
		}
		catch (std::out_of_range &) {}

		std::srand(static_cast<unsigned int>(std::time(NULL)));
		for (; portsBegin <= portsEnd; portsBegin++)
			inter.free_rand_port(portsBegin);

		auto iosrv_thread = [](asio::io_service *iosrv) {
			bool abnormally_exit;
			do
			{
				abnormally_exit = false;
				try
				{
					iosrv->run();
				}
				catch (...) { abnormally_exit = true; }
			} while (abnormally_exit);
		};
		std::shared_ptr<asio::io_service::work> main_iosrv_work = std::make_shared<asio::io_service::work>(main_io_service);
		std::shared_ptr<asio::io_service::work> misc_iosrv_work = std::make_shared<asio::io_service::work>(misc_io_service);
		std::thread main_iosrv_thread(iosrv_thread, &main_io_service);
		main_iosrv_thread.detach();
		std::thread misc_iosrv_thread(iosrv_thread, &misc_io_service);
		misc_iosrv_thread.detach();

		std::unique_ptr<msgr_proto::server> srv = std::make_unique<msgr_proto::server>
			(main_io_service, misc_io_service, inter, asio::ip::tcp::endpoint((use_v6 ? asio::ip::tcp::v6() : asio::ip::tcp::v4()), portListener));
		std::thread input_thread([]() {
			user_record user_root;
			user_root.name = "Server";
			user_root.group = user_record::CONSOLE;
			std::string command;
			/*
			while (server_on)
			{
				std::getline(std::cin, command);
				std::string ret = inter.process_command(command, user_root);
				if (!ret.empty())
					std::cout << ret << std::endl;
			}
			*/
		});
		input_thread.detach();

		std::future<void> future = exit_promise.get_future();
		future.wait();

		misc_iosrv_work.reset();
		misc_io_service.stop();

		main_iosrv_work.reset();
		main_io_service.stop();
#ifdef NDEBUG
	}
	catch (std::exception& e)
	{
		std::cerr << "Exception: " << e.what() << "\n";
	}
#endif
	return 0;
}

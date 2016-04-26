#include "stdafx.h"
#include "crypto.h"
#include "session.h"
#include "main.h"

const std::string empty_string;
const char *privatekeyFile = ".privatekey";

const std::string category_conv[] = {
	"group",
	"work",
	"member"
};

struct sql_result
{
	sql_result(MYSQL* _con) { res = mysql_store_result(_con); }
	sql_result(const sql_result &) = delete;
	sql_result(sql_result &&_res) :res(_res.res) { _res.res = nullptr; }
	~sql_result() { if (res != nullptr) mysql_free_result(res); }

	inline operator bool() { return res != nullptr; }
	inline bool operator!() { return res == nullptr; }
	inline operator MYSQL_RES*() { return res; };

	MYSQL_RES* res;
};

std::promise<void> exit_promise;
config_table_tp config_items;

asio::io_service main_io_service, misc_io_service;
lwm_server inter;
volatile bool server_on = true;

void ltrim(std::string& str)
{
	if (str.empty())
		return;
	std::string::iterator itr = str.begin(), itrEnd = str.end();
	for (; itr != itrEnd; itr++)
		if (!isspace(*itr))
			break;
	str.erase(str.begin(), itr);
}

void rtrim(std::string& str)
{
	if (str.empty())
		return;
	while (isspace(str.back()))
		str.pop_back();
}

void trim(std::string& str)
{
	ltrim(str);
	rtrim(str);
}

void lwm_server::write_data()
{
	std::ofstream fout(data_file, std::ios_base::out | std::ios_base::binary);
	if (!fout.is_open())
		return;
	fout.write(reinterpret_cast<const char*>(&data_ver), sizeof(uint32_t));
	uint32_t size = user_records.size();
	fout.write(reinterpret_cast<char*>(&size), sizeof(uint32_t));
	for (const std::pair<std::string, user_record> &pair : user_records)
	{
		const user_record &user = pair.second;
		size = user.name.size();
		fout.write(reinterpret_cast<char*>(&size), sizeof(uint32_t));
		fout.write(user.name.data(), size);
		fout.write(user.passwd.data(), hash_size);
		size = static_cast<uint32_t>(user.group);
		fout.write(reinterpret_cast<char*>(&size), sizeof(uint32_t));
	}
}

void lwm_server::read_data()
{
	if (!fs::exists(data_file))
	{
		write_data();
		return;
	}
	std::ifstream fin(data_file, std::ios_base::in | std::ios_base::binary);

	uint32_t data_file_ver;
	fin.read(reinterpret_cast<char*>(&data_file_ver), sizeof(uint32_t));
	if (data_file_ver != data_ver)
	{
		std::cout << "Incompatible data file.Will not read." << std::endl;
		return;
	}
	uint32_t userCount, size;
	fin.read(reinterpret_cast<char*>(&userCount), sizeof(uint32_t));
	char passwd_buf[hash_size];
	for (; userCount > 0; userCount--)
	{
		user_record user;
		fin.read(reinterpret_cast<char*>(&size), sizeof(uint32_t));
		std::unique_ptr<char[]> buf = std::make_unique<char[]>(size);
		fin.read(buf.get(), size);
		user.name = std::string(buf.get(), size);
		fin.read(passwd_buf, hash_size);
		user.passwd = std::string(passwd_buf, hash_size);
		fin.read(reinterpret_cast<char*>(&size), sizeof(uint32_t));
		user.group = static_cast<user_record::group_type>(size);
		user_records.emplace(user.name, user);
	}
}

void lwm_server::read_config()
{
	if (!fs::exists(config_file))
		return;
	std::ifstream fin(config_file);

	std::string line;
	std::getline(fin, line);
	while (!fin.eof())
	{
		trim(line);
		if (!line.empty() && line.front() != '#')
		{
			size_t pos = line.find('=');
			if (pos == std::string::npos)
				config_items.emplace(std::move(line), empty_string);
			else
			{
				std::string name = line.substr(0, pos), val = line.substr(pos + 1);
				rtrim(name);
				ltrim(val);
				config_items.emplace(std::move(name), std::move(val));
			}
		}
		std::getline(fin, line);
	}
}

#define checkErr(x) if (dataItr + (x) > dataEnd) throw(0)
#define read_uint(x)												\
	checkErr(size_length);											\
	memcpy(reinterpret_cast<char*>(&(x)), dataItr, size_length);	\
	dataItr += size_length

inline void write_id(std::string& result, const char* id_str)
{
	lwm_server::id_type id = static_cast<lwm_server::id_type>(std::atoi(id_str));
	result.append(reinterpret_cast<char*>(&id), sizeof(lwm_server::id_type));
}

void write_str(std::string& result, const char* str)
{
	if (str)
	{
		lwm_server::data_size_type size = strlen(str);
		result.append(reinterpret_cast<char*>(&size), sizeof(lwm_server::data_size_type));
		result.append(str);
	}
	else
	{
		lwm_server::data_size_type size = 0;
		result.append(reinterpret_cast<char*>(&size), sizeof(lwm_server::data_size_type));
	}
}

void write_list(std::string& result, const char* str)
{
	uint16_t item_count = 0;
	if (str)
	{
		std::string item, item_list;
		item_list.clear();
		for (const char *itr = str; *itr != '\0'; itr++)
		{
			if (*itr == ';')
			{
				lwm_server::id_type id = static_cast<lwm_server::id_type>(std::stoi(item));
				item_list.append(reinterpret_cast<char*>(&id), sizeof(lwm_server::id_type));
				item_count++;
				item.clear();
			}
			else
				item.push_back(*itr);
		}
		if (!item.empty())
		{
			lwm_server::id_type id = static_cast<lwm_server::id_type>(std::stoi(item));
			item_list.append(reinterpret_cast<char*>(&id), sizeof(lwm_server::id_type));
			item_count++;
		}
		result.append(reinterpret_cast<char*>(&item_count), sizeof(uint16_t));
		result.append(item_list);
	}
	else
	{
		result.append(reinterpret_cast<char*>(&item_count), sizeof(uint16_t));
	}
}

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

						send_data(id, { ERR_SUCCESS }, msgr_proto::session::priority_sys);
					}
				}

				if (user.current_stage != user_ext::LOGGED_IN)
					send_data(id, { ERR_FAILURE }, msgr_proto::session::priority_sys);

				break;
			}
			case user_ext::LOGGED_IN:
			{
				try
				{
					checkErr(1);
					char operation = *dataItr;
					dataItr++;

					std::string category_str;
					checkErr(1);
					char category = *dataItr;
					category_str = category_conv[category];
					dataItr++;

					switch (operation)
					{
						case OP_LIST:
						{
							if (mysql_query(sql_conn, ("SELECT * FROM `" + category_str + "`").c_str()) != 0)
								throw(0);
							sql_result sql_result(sql_conn);
							if (!sql_result)
							{
								throw(0);
							}
							else
							{
								MYSQL_ROW row;
								std::string result;
								result.push_back(ERR_SUCCESS);

								switch (category)
								{
									case CAT_GROUP:
									{
										while ((row = mysql_fetch_row(sql_result)) != nullptr)
										{
											write_id(result, row[0]);	//id
											write_str(result, row[1]);	//name
											write_list(result, row[2]);	//member
										}
										break;
									}
									case CAT_WORK:
									{
										while ((row = mysql_fetch_row(sql_result)) != nullptr)
										{
											write_id(result, row[0]);	//id
											write_str(result, row[1]);	//name
											write_str(result, row[2]);	//info
											write_list(result, row[3]);	//member
										}
										break;
									}
									case CAT_MEMBER:
									{
										while ((row = mysql_fetch_row(sql_result)) != nullptr)
										{
											write_id(result, row[0]);	//id
											write_str(result, row[1]);	//name
											write_list(result, row[2]);	//group
											write_list(result, row[3]);	//work
											write_str(result, row[4]);	//src
											write_str(result, row[5]);	//info
										}
										break;
									}
									default:
										throw(0);
								}

								if (mysql_errno(sql_conn) != 0)
									throw(0);
								send_data(id, result, msgr_proto::session::priority_sys);
							}
							break;
						}
						default:
							throw(0);
					}
				}
				catch (...) { send_data(id, { ERR_FAILURE }, msgr_proto::session::priority_sys); throw; }

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

std::string lwm_server::process_command(std::string cmd, user_record& user)
{
	user_record::group_type group = user.group;
	std::string ret;

	int pos = cmd.find(' ');
	std::string args;
	if (pos != std::string::npos)
	{
		args.assign(cmd, pos + 1, std::string::npos);
		cmd.erase(pos);
	}
	trim(args);

	if (cmd == "op")
	{
		if (group >= user_record::ADMIN)
		{
			user_record_list::iterator itr = user_records.find(args);
			if (itr != user_records.end())
			{
				itr->second.group = user_record::ADMIN;
				main_io_service.post([this]() {
					write_data();
				});
				ret = "Opped " + itr->second.name;
			}
		}
	}
	else if (cmd == "reg")
	{
		if (group >= user_record::ADMIN)
		{
			pos = args.find(' ');
			if (pos != std::string::npos)
			{
				cmd = args.substr(0, pos);
				args.erase(0, pos);
				trim(cmd);
				trim(args);
				std::string hashed_passwd;
				hash(args, hashed_passwd);

				user_record_list::iterator itr = user_records.find(cmd);
				if (itr == user_records.end())
				{
					user_records.emplace(cmd, user_record(cmd, hashed_passwd, user_record::USER));
					main_io_service.post([this]() {
						write_data();
					});
					ret = "Registered " + cmd;
				}
			}
		}
	}
	else if (cmd == "unreg")
	{
		if (group >= user_record::ADMIN)
		{
			user_record_list::iterator itr = user_records.find(args);
			if (itr != user_records.end())
			{
				user_records.erase(itr);
				main_io_service.post([this]() {
					write_data();
				});
				ret = "Unregistered " + args;
			}
		}
	}
	else if (cmd == "changepass")
	{
		user.passwd.clear();
		hash(args, user.passwd);
		main_io_service.post([this]() {
			write_data();
		});
		ret = "Password changed";
	}
	else if (cmd == "stop")
	{
		if (group >= user_record::CONSOLE)
		{
			server_on = false;
			exit_promise.set_value();
			ret = "Stopping server";
		}
	}
	return ret;
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

bool lwm_server::init_sql_conn()
{
	try
	{
		std::string &addr = config_items.at("sql_addr");
		std::string &sql_port_str = config_items.at("sql_port");
		port_type sql_port = static_cast<port_type>(std::stoi(sql_port_str));
		std::string &user = config_items.at("sql_user");
		std::string &pass = config_items.at("sql_pass");
		std::string &db_name = config_items.at("sql_db");

		MYSQL *conn = mysql_init(nullptr);
		if (!conn)
			throw(std::runtime_error("Failed to init connect to SQL"));
		conn = mysql_real_connect(conn, addr.c_str(), user.c_str(), pass.c_str(), db_name.c_str(), sql_port, nullptr, 0);
		if (!conn)
			throw(std::runtime_error("Failed to connect to SQL"));

		sql_conn = conn;
		std::cout << "Connected to SQL server at " << addr << ':' << sql_port << std::endl;
	}
	catch (std::out_of_range &) { return false; }
	catch (std::invalid_argument &) { return false; }
	return true;
}

int main(int argc, char *argv[])
{
	try
	{
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
		if (!inter.init_sql_conn())
		{
			std::cerr << "SQL connection arg not set or invalid" << std::endl;
			throw(0);
		}

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
			while (server_on)
			{
				std::getline(std::cin, command);
				std::string ret = inter.process_command(command, user_root);
				if (!ret.empty())
					std::cout << ret << std::endl;
			}
		});
		input_thread.detach();

		std::future<void> future = exit_promise.get_future();
		future.wait();

		misc_iosrv_work.reset();
		misc_io_service.stop();

		main_iosrv_work.reset();
		main_io_service.stop();
	}
	catch (int)
	{
		return EXIT_FAILURE;
	}
	catch (std::exception& e)
	{
		std::cerr << "Exception: " << e.what() << std::endl;
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}

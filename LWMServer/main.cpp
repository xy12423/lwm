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

struct data_view
{
	data_view(const char* _data, size_t _size)
		:data(_data), size(_size)
	{}
	data_view(const std::string &_data)
		:data(_data.data()), size(_data.size())
	{}

	template <typename _Ty>
	inline bool read(_Ty &ret) {
		if (size < sizeof(_Ty))
			return false;
		size -= sizeof(_Ty);
		ret = *reinterpret_cast<const _Ty*>(data);
		data += sizeof(_Ty);
		return true;
	}
	inline bool read(char* dst, size_t _size) { if (size < _size) return false; memcpy(dst, data, _size); data += _size; size -= _size;  return true; }
	inline bool read(std::string& dst, size_t _size) { if (size < _size) return false; dst.append(data, _size); data += _size; size -= _size;  return true; }
	inline bool check(size_t count) { return size >= count; }
	inline bool skip(size_t count) { if (size < count) return false; data += count; size -= count; return true; }

	const char* data;
	size_t size;
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
	id = boost::endian::native_to_little(id);
	result.append(reinterpret_cast<char*>(&id), sizeof(lwm_server::id_type));
}

inline void write_id(std::string& result, lwm_server::id_type id)
{
	id = boost::endian::native_to_little(id);
	result.append(reinterpret_cast<char*>(&id), sizeof(lwm_server::id_type));
}

void write_str(std::string& result, const char* str)
{
	if (str)
	{
		lwm_server::data_size_type size = strlen(str);
		size = boost::endian::native_to_little(size);
		result.append(reinterpret_cast<char*>(&size), sizeof(lwm_server::data_size_type));
		result.append(str);
	}
	else
	{
		lwm_server::data_size_type size = 0;
		size = boost::endian::native_to_little(size);
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
				id = boost::endian::native_to_little(id);
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
			id = boost::endian::native_to_little(id);
			item_list.append(reinterpret_cast<char*>(&id), sizeof(lwm_server::id_type));
			item_count++;
		}
		item_count = boost::endian::native_to_little(item_count);
		result.append(reinterpret_cast<char*>(&item_count), sizeof(uint16_t));
		result.append(item_list);
	}
	else
	{
		result.append(reinterpret_cast<char*>(&item_count), sizeof(uint16_t));
	}
}

void write_list(std::set<lwm_server::id_type>& result, const char* str)
{
	if (str)
	{
		std::string item;
		for (const char *itr = str; *itr != '\0'; itr++)
		{
			if (*itr == ';')
			{
				lwm_server::id_type id = static_cast<lwm_server::id_type>(std::stoi(item));
				result.insert(id);
				item.clear();
			}
			else
				item.push_back(*itr);
		}
		if (!item.empty())
		{
			lwm_server::id_type id = static_cast<lwm_server::id_type>(std::stoi(item));
			result.insert(id);
		}
	}
}

void read_list(std::string& result, const std::set<lwm_server::id_type>& src)
{
	for (std::set<lwm_server::id_type>::const_iterator itr = src.cbegin(), itrEnd = src.cend(); itr != itrEnd; itr++)
	{
		result.append(std::to_string(*itr));
		result.push_back(';');
	}
}

void read_list(std::string& result, data_view& src)
{
	uint16_t item_count;
	if (!src.read(item_count)) throw(0);
	item_count = boost::endian::little_to_native(item_count);

	for (uint16_t i = 0; i < item_count; i++)
	{
		lwm_server::id_type item;
		if (!src.read(item)) throw(0);
		item = boost::endian::little_to_native(item);
		result.append(std::to_string(item));
		result.push_back(';');
	}
}

void lwm_server::on_data(user_id_type id, const std::string &data)
{
	try
	{
		user_ext &user = user_exts.at(id);

		const size_t size_length = sizeof(data_size_type);
		const char *dataItr = data.data(), *dataEnd = data.data() + data.size();

		switch (user.current_stage)
		{
			case user_ext::LOGGING_IN:
			{
				data_size_type name_size, pass_size;
				
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
					sql_conn_tp sql_conn;
					sql_connect(sql_conn);

					checkErr(1);
					char operation = *dataItr;
					dataItr++;

					std::string category_str;
					checkErr(1);
					char category = *dataItr;
					category_str = category_conv[category];
					dataItr++;

					std::string result;

					switch (operation)
					{
						case OP_LIST:
						{
							if (mysql_query(sql_conn, ("SELECT * FROM `" + category_str + "`").c_str()) != 0)
								throw(0);
							sql_result_tp sql_result(sql_conn);
							if (!sql_result)
								throw(0);

							MYSQL_ROW row;
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
										char status = static_cast<char>(row[6] == nullptr ? 0 : std::atoi(row[6]));
										result.push_back(status);
									}
									break;
								}
								default:
									throw(0);
							}
							break;
						}
						case OP_ADD:
						{
							std::string new_name(dataItr, dataEnd - dataItr);

							if (mysql_query(sql_conn, ("SELECT `count` FROM `data` WHERE `name`='" + category_str + '\'').c_str()) != 0)
								throw(0);
							sql_result_tp sql_result(sql_conn);
							if (!sql_result)
								throw(0);

							MYSQL_ROW row = mysql_fetch_row(sql_result);
							if (row == nullptr)
								throw(0);
							id_type new_id = static_cast<lwm_server::id_type>(std::atoi(row[0]));
							sql_result.reset();

							if (mysql_query(sql_conn, ("UPDATE `data` SET `count`=" + std::to_string(new_id + 1) + " WHERE `name`='" + category_str + '\'').c_str()) != 0)
								throw(0);

							if (mysql_query(sql_conn, ("INSERT INTO `" + category_str + "` (`id`, `name`) VALUES (" + std::to_string(new_id) + ",'" + new_name + "')").c_str()) != 0)
								throw(0);

							result.push_back(ERR_SUCCESS);
							write_id(result, new_id);

							break;
						}
						case OP_DEL:
						{
							id_type id;
							checkErr(sizeof(id_type));
							memcpy(reinterpret_cast<char*>(&id), dataItr, sizeof(id_type));
							id = boost::endian::little_to_native(id);
							dataItr += sizeof(id_type);

							if (mysql_query(sql_conn, ("SELECT * FROM `" + category_str + "` WHERE `id`=" + std::to_string(id)).c_str()) != 0)
								throw(0);
							sql_result_tp sql_result(sql_conn);
							if (!sql_result)
								throw(0);
							MYSQL_ROW row = mysql_fetch_row(sql_result);

							if (category == CAT_MEMBER)
							{
								std::set<id_type> group_list, work_list, member_list;
								std::string new_member_list;
								write_list(group_list, row[2]);
								write_list(work_list, row[3]);
								sql_result.reset();

								for (id_type gID : group_list)
								{
									if (mysql_query(sql_conn, ("SELECT `member` FROM `group` WHERE `id`=" + std::to_string(gID)).c_str()) != 0)
										throw(0);
									sql_result.reset(sql_conn);
									row = mysql_fetch_row(sql_result);

									member_list.clear();
									write_list(member_list, row[0]);
									member_list.erase(id);
									new_member_list.clear();
									read_list(new_member_list, member_list);

									sql_result.reset();
									if (mysql_query(sql_conn, ("UPDATE `group` SET `member`='" + new_member_list + "' WHERE `id`=" + std::to_string(gID)).c_str()) != 0)
										throw(0);
								}

								for (id_type wID : work_list)
								{
									if (mysql_query(sql_conn, ("SELECT `member` FROM `work` WHERE `id`=" + std::to_string(wID)).c_str()) != 0)
										throw(0);
									sql_result.reset(sql_conn);
									row = mysql_fetch_row(sql_result);

									member_list.clear();
									write_list(member_list, row[0]);
									member_list.erase(id);
									new_member_list.clear();
									read_list(new_member_list, member_list);

									sql_result.reset();
									if (mysql_query(sql_conn, ("UPDATE `work` SET `member`='" + new_member_list + "' WHERE `id`=" + std::to_string(wID)).c_str()) != 0)
										throw(0);
								}
							}
							else
							{
								std::set<id_type> member_list, parent_list;
								std::string new_parent_list;
								if (category == CAT_GROUP)
									write_list(member_list, row[2]);
								else
									write_list(member_list, row[3]);
								sql_result.reset();

								for (id_type uID : member_list)
								{
									if (mysql_query(sql_conn, ("SELECT `" + category_str + "` FROM `member` WHERE `id`=" + std::to_string(uID)).c_str()) != 0)
										throw(0);
									sql_result.reset(sql_conn);
									row = mysql_fetch_row(sql_result);

									parent_list.clear();
									if (category == CAT_GROUP)
										write_list(parent_list, row[0]);
									else
										write_list(parent_list, row[0]);
									parent_list.erase(id);
									new_parent_list.clear();
									read_list(new_parent_list, parent_list);

									sql_result.reset();
									if (mysql_query(sql_conn, ("UPDATE `member` SET `" + category_str + "`='" + new_parent_list + "' WHERE `id`=" + std::to_string(uID)).c_str()) != 0)
										throw(0);
								}
							}

							if (mysql_query(sql_conn, ("DELETE FROM `" + category_str + "` WHERE `id`=" + std::to_string(id)).c_str()) != 0)
								throw(0);

							result.push_back(ERR_SUCCESS);

							break;
						}
						case OP_MODIFY:
						{
							data_view new_data(dataItr, dataEnd - dataItr);
							id_type id;
							new_data.read(id);
							id = boost::endian::little_to_native(id);
							switch (category)
							{
								case CAT_GROUP:
								{
									data_size_type str_size;
									std::string name, member;
									if (!new_data.read(str_size)) throw(0);
									if (!new_data.read(name, str_size)) throw(0);

									read_list(member, new_data);

									if (mysql_query(sql_conn, ("UPDATE `" + category_str + "` SET `name`='" + name + "', `member`='" + member + "' WHERE `id`=" + std::to_string(id)).c_str()) != 0)
										throw(0);

									break;
								}
								case CAT_WORK:
								{
									data_size_type str_size;
									std::string name, info, member;
									if (!new_data.read(str_size)) throw(0);
									if (!new_data.read(name, str_size)) throw(0);
									if (!new_data.read(str_size)) throw(0);
									if (!new_data.read(info, str_size)) throw(0);

									read_list(member, new_data);

									if (mysql_query(sql_conn, ("UPDATE `" + category_str + "` SET `name`='" + name + "', `info`='" + info + "', `member`='" + member + "' WHERE `id`=" + std::to_string(id)).c_str()) != 0)
										throw(0);

									break;
								}
								case CAT_MEMBER:
								{
									data_size_type str_size;
									std::string name, src, info, group, work;
									char status;
									if (!new_data.read(str_size)) throw(0);
									if (!new_data.read(name, str_size)) throw(0);

									read_list(group, new_data);
									read_list(work, new_data);

									if (!new_data.read(str_size)) throw(0);
									if (!new_data.read(src, str_size)) throw(0);
									if (!new_data.read(str_size)) throw(0);
									if (!new_data.read(info, str_size)) throw(0);
									if (!new_data.read(status)) throw(0);

									if (mysql_query(sql_conn, (
										"UPDATE `" + category_str + "` SET `name`='" + name + 
										"', `group`='" + group + "', `work`='" + work + 
										"', `src`='" + src + "', `info`='" + info + "', `status`=" + std::to_string(static_cast<int>(status)) + 
										" WHERE `id`=" + std::to_string(id)
										).c_str()) != 0)
										throw(0);

									break;
								}
								default:
									throw(0);
							}

							result.push_back(ERR_SUCCESS);
							break;
						}
						default:
							throw(0);
					}
					send_data(id, result, msgr_proto::session::priority_sys);
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

bool lwm_server::sql_connect(sql_conn_tp& sql_conn)
{
	try
	{
		std::string &addr = config_items.at("sql_addr");
		std::string &sql_port_str = config_items.at("sql_port");
		port_type sql_port = static_cast<port_type>(std::stoi(sql_port_str));
		std::string &user = config_items.at("sql_user");
		std::string &pass = config_items.at("sql_pass");
		std::string &db_name = config_items.at("sql_db");

		sql_conn.connect(addr, sql_port, user, pass, db_name);
		if (!sql_conn)
			throw(std::runtime_error("Failed to connect to SQL"));

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
		sql_conn_tp test_conn;
		if (!inter.sql_connect(test_conn))
		{
			std::cerr << "SQL connection arg not set or invalid" << std::endl;
			throw(0);
		}
		test_conn.reset();

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

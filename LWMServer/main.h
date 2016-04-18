#pragma once

#ifndef _H_MAIN
#define _H_MAIN

struct user_record
{
	enum group_type { GUEST, USER, ADMIN, CONSOLE };

	user_record() { group = GUEST; }
	user_record(const std::string &_name, const std::string &_passwd, group_type _group) :
		name(_name), passwd(_passwd)
	{
		group = _group;
	}

	std::string name, passwd;
	group_type group;
	bool logged_in;
	user_id_type id;
};
typedef std::unordered_map<std::string, user_record> user_record_list;

struct user_ext
{
	enum stage { LOGGING_IN, LOGGED_IN };
	stage current_stage = LOGGING_IN;

	std::string name;
};
typedef std::unordered_map<int, user_ext> user_ext_list;

const std::string server_uname = "Server";
const int server_uid = -1;
class lwm_server :public server_interface
{
public:
	lwm_server() {
		read_config();
		read_data();
		user_exts[server_uid].name = server_uname;
		user_exts[server_uid].current_stage = user_ext::LOGGED_IN;
		initKey();
	}
	~lwm_server() {
		write_data();
	}

	virtual void on_data(user_id_type id, const std::string &data);

	virtual void on_join(user_id_type id, const std::string &);
	virtual void on_leave(user_id_type id);

	virtual bool new_rand_port(port_type &port);
	virtual void free_rand_port(port_type port) { ports.push_back(port); };

	inline void send_data(user_id_type id, const std::string &data, int priority) { srv->send_data(id, data, priority); };
	void broadcast_data(int id, const std::string &data, int priority);

	std::string process_command(std::string cmd, user_record& user);

	void set_static_port(port_type port) { static_port = port; };
private:
	void read_data();
	void write_data();
	void read_config() {};

	const char *config_file = ".config";
	const char *data_file = ".data";
	const uint32_t data_ver = 0x00;

	int static_port = -1;
	std::list<port_type> ports;

	user_record_list user_records;
	user_ext_list user_exts;
};

typedef std::unordered_map<std::string, std::string> config_table_tp;

#endif

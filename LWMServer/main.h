#pragma once

#ifndef _H_MAIN
#define _H_MAIN

struct sql_conn_tp
{
	sql_conn_tp() :conn(nullptr) {}
	sql_conn_tp(const std::string& addr, port_type port, const std::string& user, const std::string& pass, const std::string& db_name)
	{
		connect(addr, port, user, pass, db_name);
	}
	sql_conn_tp(const sql_conn_tp&) = delete;
	sql_conn_tp(sql_conn_tp&& _res) :conn(_res.conn) { _res.conn = nullptr; }
	~sql_conn_tp() { if (conn != nullptr) mysql_close(conn); }

	inline void reset(){ if (conn != nullptr) { mysql_close(conn); conn = nullptr; } }
	inline void connect(const std::string& addr, port_type port, const std::string& user, const std::string& pass, const std::string& db_name)
	{
		reset();
		conn = mysql_init(nullptr);
		if (conn == nullptr)
			return;
		conn = mysql_real_connect(conn, addr.c_str(), user.c_str(), pass.c_str(), db_name.c_str(), port, nullptr, 0);
		if (conn == nullptr)
			return;
	}

	inline operator bool() { return conn != nullptr; }
	inline bool operator!() { return conn == nullptr; }
	inline operator MYSQL*() { return conn; };

	MYSQL *conn;
};

struct sql_result_tp
{
	sql_result_tp(MYSQL* _con) { res = mysql_store_result(_con); }
	sql_result_tp(const sql_result_tp&) = delete;
	sql_result_tp(sql_result_tp&& _res) :res(_res.res) { _res.res = nullptr; }
	~sql_result_tp() { if (res != nullptr) mysql_free_result(res); }

	inline operator bool() { return res != nullptr; }
	inline bool operator!() { return res == nullptr; }
	inline operator MYSQL_RES*() { return res; };

	void reset(MYSQL* _con = nullptr) {
		if (res != nullptr) { mysql_free_result(res); res = nullptr; }
		if (_con != nullptr) { res = mysql_store_result(_con); }
	}

	MYSQL_RES* res;
};

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
class lwm_server :public msgr_inter
{
public:
	enum predefined_err {
		ERR_SUCCESS,
		ERR_FAILURE,
		ERR_DISCONNECTED,
		ERR_TIMED_OUT,
	};
	typedef uint8_t err_t;

	enum operation_t {
		OP_LIST,
		OP_ADD,
		OP_DEL,
		OP_INFO,
		OP_MODIFY,
	};

	enum category_t {
		CAT_GROUP,
		CAT_WORK,
		CAT_MEMBER,
	};

	typedef uint16_t id_type;
	typedef uint32_t data_size_type;

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
	bool sql_connect(sql_conn_tp& sql_conn);
private:
	void read_data();
	void write_data();
	void read_config();

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

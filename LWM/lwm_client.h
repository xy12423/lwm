#pragma once

#ifndef _H_LWM_CLIENT
#define _H_LWM_CLIENT

#include "session.h"

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
	bool skip(size_t count) { if (size < count) return false; data += count; size -= count; return true; }

	const char* data;
	size_t size;
};

class lwm_client :public msgr_inter
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

		OP_LOGIN
	};

	enum category_t {
		CAT_GROUP,
		CAT_WORK,
		CAT_MEMBER,

		CAT_NOCAT
	};

	typedef uint16_t id_type;
	typedef uint32_t data_size_type;

	struct request
	{
		request() :cat(CAT_NOCAT) {}
		request(operation_t _op, category_t _cat, id_type _id)
			:op(_op), cat(_cat), id(_id)
		{}
		operation_t op;
		category_t cat;
		id_type id;
	};

	struct response
	{
		response(err_t _err, const std::string& _data)
			:err(_err), data(_data)
		{}
		response(const request& _req, err_t _err, const char* _data, size_t _size)
			:err(_err), data(_data, _size)
		{}
		request req;
		err_t err;
		data_view data;
	};

	typedef std::function<void(response)> lwm_callback;

	virtual void on_data(user_id_type id, const std::string& data);

	virtual void on_join(user_id_type id, const std::string& key);
	virtual void on_leave(user_id_type id);

	virtual bool new_rand_port(port_type& port);
	virtual void free_rand_port(port_type port) { ports.push_back(port); };

	void connect(const std::string& addr, port_type remote_port) { srv->connect(addr, remote_port); };
	void login(const std::string& name, const std::string& pass);

	void list(category_t cat);
	void add(category_t cat, const std::string& name);
	void del(category_t cat, id_type id);
	void info(category_t cat, id_type id);
	void modify(category_t cat, id_type id, const std::string& data);

	std::string get_server_key() { return server_key; };

	void set_callback(lwm_callback&& _callback) { callback = std::make_shared<lwm_callback>(_callback); };
private:
	void call_callback(const response& resp) { if (callback) { std::shared_ptr<lwm_callback> _callback = callback; (*callback)(resp); } };

	std::list<port_type> ports;
	std::string server_key;
	user_id_type server_id;

	std::list<request> pending;

	std::shared_ptr<lwm_callback> callback;
};

#endif

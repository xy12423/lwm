#include "stdafx.h"
#include "lwm_client.h"

const std::string empty_string;

void lwm_client::on_data(user_id_type id, const std::string& data)
{
	assert(id == server_id);
	call_callback(response(pending.front(), data.front(), data.data(), data.size() - 1));
	pending.pop_front();
}

void lwm_client::on_join(user_id_type id, const std::string& key)
{
	if (!key.empty())
	{
		server_id = id;
		server_key = key;
		call_callback(response(ERR_SUCCESS, key));
	}
}

void lwm_client::on_leave(user_id_type id)
{
	server_key.clear();
	pending.clear();
	call_callback(response(ERR_DISCONNECTED, empty_string));
}

void lwm_client::login(const std::string& name, const std::string& pass)
{
	std::string data;
	data_length_type name_len = boost::endian::native_to_little(static_cast<data_length_type>(name.size())),
		pass_len = boost::endian::native_to_little(static_cast<data_length_type>(pass.size()));
	data.append(reinterpret_cast<const char*>(&name_len), sizeof(data_length_type));
	data.append(name);
	data.append(reinterpret_cast<const char*>(&pass_len), sizeof(data_length_type));
	data.append(pass);

	pending.push_back(request(OP_LOGIN, CAT_NOCAT, 0));
	srv->send_data(server_id, data, msgr_proto::session::priority_sys);
}

bool lwm_client::new_rand_port(port_type& ret)
{
	if (ports.empty())
		return false;
	std::list<port_type>::iterator portItr = ports.begin();
	for (int i = std::rand() % ports.size(); i > 0; i--)
		portItr++;
	ret = *portItr;
	ports.erase(portItr);

	return true;
}

void lwm_client::list(category_t cat)
{
	std::string req;
	req.push_back(OP_LIST);
	req.push_back(cat);
	pending.push_back(request(OP_LIST, cat, 0));
	srv->send_data(server_id, std::move(req), msgr_proto::session::priority_msg);
}

void lwm_client::add(category_t cat, const std::string& name)
{
	std::string req;
	req.push_back(OP_ADD);
	req.push_back(cat);
	req.append(name);
	pending.push_back(request(OP_ADD, cat, 0));
	srv->send_data(server_id, std::move(req), msgr_proto::session::priority_msg);
}

void lwm_client::del(category_t cat, id_type id)
{
	std::string req;
	req.push_back(OP_DEL);
	req.push_back(cat);
	id_type _id = boost::endian::native_to_little(id);
	req.append(reinterpret_cast<char*>(&_id), sizeof(id_type));
	pending.push_back(request(OP_DEL, cat, id));
	srv->send_data(server_id, std::move(req), msgr_proto::session::priority_msg);
}

void lwm_client::info(category_t cat, id_type id)
{
	std::string req;
	req.push_back(OP_INFO);
	req.push_back(cat);
	id_type _id = boost::endian::native_to_little(id);
	req.append(reinterpret_cast<char*>(&_id), sizeof(id_type));
	pending.push_back(request(OP_INFO, cat, id));
	srv->send_data(server_id, std::move(req), msgr_proto::session::priority_msg);
}

void lwm_client::modify(category_t cat, id_type id, const std::string& data)
{
	std::string req;
	req.push_back(OP_MODIFY);
	req.push_back(cat);
	id_type _id = boost::endian::native_to_little(id);
	req.append(reinterpret_cast<char*>(&_id), sizeof(id_type));
	req.append(data);
	pending.push_back(request(OP_MODIFY, cat, id));
	srv->send_data(server_id, std::move(req), msgr_proto::session::priority_msg);
}

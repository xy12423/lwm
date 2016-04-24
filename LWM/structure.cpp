#include "stdafx.h"
#include "structure.h"

extern lwm_client client;

grpListTp grpList;
memListTp memList;
workListTp workList;

void load_group(id_type id, data_view& data)
{
	data_size_type name_size;
	data.read(name_size);
	std::wstring name(wxConvUTF8.cMB2WC(data.data));
	data.skip(name_size);

	group &grp = grpList.try_emplace(id, group(id, std::move(name))).first->second;

	uint16_t user_count;
	id_type uid;
	data.read(user_count);
	grp.members.clear();
	for (int i = 0; i < user_count; i++)
	{
		data.read(uid);
		grp.members.emplace(uid);
	}
}

void load_groups(data_view& data)
{
	id_type id;
	while (data.size != 0)
	{
		if (!data.read(id))
			break;
		load_group(id, data);
	}
}

lwm_client::err_t list_group()
{
	std::promise<lwm_client::err_t> list_promise;
	std::future<lwm_client::err_t> list_future = list_promise.get_future();
	client.set_callback([&list_promise](lwm_client::response response) {
		if (response.err == lwm_client::ERR_SUCCESS)
			load_groups(response.data);
		list_promise.set_value(response.err);
	});
	client.list(lwm_client::CAT_GROUP);

	lwm_client::err_t err = list_future.get();
	if (err != lwm_client::ERR_SUCCESS)
	{
		return err;
	}
	return lwm_client::ERR_SUCCESS;
}

void group::submit()
{
	std::string data;
	std::string name_utf8(wxConvUTF8.cWC2MB(name.c_str()));
	uint32_t name_len = boost::endian::native_to_little(static_cast<uint32_t>(name_utf8.size()));
	data.append(reinterpret_cast<char*>(&name_len), sizeof(uint32_t));
	data.append(name_utf8);

	uint16_t user_count = boost::endian::native_to_little(static_cast<uint16_t>(members.size()));
	data.append(reinterpret_cast<char*>(&user_count), sizeof(uint16_t));
	for (id_type ID : members)
		data.append(reinterpret_cast<char*>(&ID), sizeof(id_type));

	client.modify(lwm_client::CAT_GROUP, gID, data);
}

void member::submit()
{
	std::string data;
	std::string str_utf8(wxConvUTF8.cWC2MB(name.c_str()));
	uint32_t str_len = boost::endian::native_to_little(static_cast<uint32_t>(str_utf8.size()));
	data.append(reinterpret_cast<char*>(&str_len), sizeof(uint32_t));
	data.append(str_utf8);

	str_utf8 = wxConvUTF8.cWC2MB(extInfo.src.c_str());
	str_len = boost::endian::native_to_little(static_cast<uint32_t>(str_utf8.size()));
	data.append(reinterpret_cast<char*>(&str_len), sizeof(uint32_t));
	data.append(str_utf8);

	str_utf8 = wxConvUTF8.cWC2MB(extInfo.info.c_str());
	str_len = boost::endian::native_to_little(static_cast<uint32_t>(str_utf8.size()));
	data.append(reinterpret_cast<char*>(&str_len), sizeof(uint32_t));
	data.append(str_utf8);

	uint16_t group_count = boost::endian::native_to_little(static_cast<uint16_t>(groups.size()));
	data.append(reinterpret_cast<char*>(&group_count), sizeof(uint16_t));
	for (id_type ID : groups)
		data.append(reinterpret_cast<char*>(&ID), sizeof(id_type));

	uint16_t work_count = boost::endian::native_to_little(static_cast<uint16_t>(works.size()));
	data.append(reinterpret_cast<char*>(&work_count), sizeof(uint16_t));
	for (id_type ID : works)
		data.append(reinterpret_cast<char*>(&ID), sizeof(id_type));

	client.modify(lwm_client::CAT_MEMBER, uID, data);
}

void work::submit()
{
	std::string data;
	std::string str_utf8(wxConvUTF8.cWC2MB(name.c_str()));
	uint32_t str_len = boost::endian::native_to_little(static_cast<uint32_t>(str_utf8.size()));
	data.append(reinterpret_cast<char*>(&str_len), sizeof(uint32_t));
	data.append(str_utf8);

	str_utf8 = wxConvUTF8.cWC2MB(info.c_str());
	str_len = boost::endian::native_to_little(static_cast<uint32_t>(str_utf8.size()));
	data.append(reinterpret_cast<char*>(&str_len), sizeof(uint32_t));
	data.append(str_utf8);

	uint16_t user_count = boost::endian::native_to_little(static_cast<uint16_t>(members.size()));
	data.append(reinterpret_cast<char*>(&user_count), sizeof(uint16_t));
	for (id_type ID : members)
		data.append(reinterpret_cast<char*>(&ID), sizeof(id_type));

	client.modify(lwm_client::CAT_WORK, wID, data);
}

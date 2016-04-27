#include "stdafx.h"
#include "structure.h"

extern lwm_client client;

grpListTp grpList;
memListTp memList;
workListTp workList;

void read_str(data_view& data, std::wstring& ret)
{
	data_size_type str_size;
	size_t wstr_size = 0;
	if (!data.read(str_size)) throw(0);
	const wxWCharBuffer &wstr = wxConvUTF8.cMB2WC(data.data, str_size, &wstr_size);
	ret.assign(wstr, wstr_size);
	if (!data.skip(str_size)) throw(0);
}

void read_list(data_view& data, std::set<id_type>& ret)
{
	uint16_t item_count;
	id_type id;
	if (!data.read(item_count)) throw(0);
	ret.clear();
	for (int i = 0; i < item_count; i++)
	{
		if (!data.read(id)) throw(0);
		ret.emplace(id);
	}
}

void load_group(id_type id, data_view& data)
{
	std::wstring name;
	read_str(data, name);

	group &grp = grpList.try_emplace(id, group(id, std::move(name))).first->second;

	read_list(data, grp.members);
}

void load_work(id_type id, data_view& data)
{
	std::wstring name, info;
	read_str(data, name);
	read_str(data, info);

	work &wrk = workList.try_emplace(id, work(id, std::move(name), std::move(info))).first->second;

	read_list(data, wrk.members);
}

void load_member(id_type id, data_view& data)
{
	std::wstring name;
	read_str(data, name);

	member &mem = memList.try_emplace(id, member(id, std::move(name), uExtInfo())).first->second;

	read_list(data, mem.groups);
	read_list(data, mem.works);

	std::wstring src, info;
	read_str(data, src);
	read_str(data, info);

	mem.extInfo.src = std::move(src);
	mem.extInfo.info = std::move(info);
}

void load_list(data_view& data, lwm_client::category_t cat)
{
	id_type id;
	while (data.size != 0)
	{
		if (!data.read(id))
			break;
		switch (cat)
		{
			case lwm_client::CAT_GROUP:
				load_group(id, data);
				break;
			case lwm_client::CAT_WORK:
				load_work(id, data);
				break;
			case lwm_client::CAT_MEMBER:
				load_member(id, data);
				break;
		}
	}
}

lwm_client::err_t list(lwm_client::category_t cat)
{
	std::promise<lwm_client::err_t> list_promise;
	std::future<lwm_client::err_t> list_future = list_promise.get_future();
	client.set_callback([&list_promise, cat](lwm_client::response response) {
		if (response.err == lwm_client::ERR_SUCCESS)
		{
			try
			{
				load_list(response.data, cat);
			}
			catch (int) { response.err = lwm_client::ERR_FAILURE; }
		}
		list_promise.set_value(response.err);
	});
	client.list(cat);

	return list_future.get();
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

#include "stdafx.h"
#include "structure.h"

extern lwm_client client;

grpListTp grpList;
memListTp memList;
workListTp workList;

void processEscChar(std::wstring &str)
{
	std::wstring::iterator itr = str.begin();
	for (; itr != str.end(); itr++)
	{
		if (*itr == '\\')
		{
			char replace = '\0';
			itr = str.erase(itr);
			if (itr == str.end())
				break;
			switch (*itr)
			{
				case 'a':
					replace = '\a';
					break;
				case 'b':
					replace = '\b';
					break;
				case 'f':
					replace = '\f';
					break;
				case 'n':
					replace = '\n';
					break;
				case 'r':
					replace = '\r';
					break;
				case 't':
					replace = '\t';
					break;
				case 'v':
					replace = '\v';
					break;
				case '\\':
					replace = '\\';
					break;
				case '\'':
					replace = '\'';
					break;
				case '\"':
					replace = '\"';
					break;
				case 'x':
				{
					std::stringstream tmp;
					tmp << std::hex;
					for (int i = 0; i < 2 && itr != str.end(); i++)
					{
						itr = str.erase(itr);
						if (!isxdigit(*itr))
							break;
						tmp << *itr;
					}
					itr = str.insert(itr, '\0');
					int tmpn = 0;
					tmp >> tmpn;
					replace = static_cast<char>(tmpn);
					break;
				}
				default:
				{
					if (*itr > '7' || *itr < '0')
						return;
					std::stringstream tmp;
					tmp << std::oct;
					for (int i = 0; i < 3 && itr != str.end(); i++)
					{
						if (*itr > '7' || *itr < '0')
							break;
						tmp << *itr;
						itr = str.erase(itr);
					}
					itr = str.insert(itr, '\0');
					int tmpn = 0;
					tmp >> tmpn;
					replace = static_cast<char>(tmpn);
				}
			}

			*itr = replace;
		}
	}
}

void toSingleLine(std::string &str)
{
	size_t pos = str.find('\n');
	while (pos != std::string::npos)
	{
		str.replace(pos, 1, "\\n");
		pos = str.find('\n', pos + 2);
	}
}

void read_str(data_view& data, std::wstring& ret)
{
	data_size_type str_size;
	size_t wstr_size = 0;
	if (!data.read(str_size)) throw(0);
	const wxWCharBuffer &wstr = wxConvUTF8.cMB2WC(data.data, str_size, &wstr_size);
	ret.assign(wstr, wstr_size);
	if (!data.skip(str_size)) throw(0);
}

void read_list(data_view& data, std::set<id_type_l>& ret)
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
	processEscChar(info);

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
	std::shared_ptr<std::promise<lwm_client::err_t>> list_promise = std::make_shared<std::promise<lwm_client::err_t>>();
	std::future<lwm_client::err_t> list_future = list_promise->get_future();
	client.set_callback([list_promise, cat](lwm_client::response response) {
		if (response.err == lwm_client::ERR_SUCCESS)
		{
			try
			{
				load_list(response.data, cat);
			}
			catch (int) { response.err = lwm_client::ERR_FAILURE; }
		}
		list_promise->set_value(response.err);
	});
	client.list(cat);

	return list_future.get();
}

lwm_client::err_t add(lwm_client::category_t cat, const std::wstring& name, id_type& ret)
{
	std::shared_ptr<std::promise<int>> add_promise = std::make_shared<std::promise<int>>();
	std::future<int> add_future = add_promise->get_future();
	client.set_callback([add_promise](lwm_client::response response) {
		if (response.err == lwm_client::ERR_SUCCESS)
		{
			id_type id;
			response.data.read(id);
			add_promise->set_value(id);
		}
		else
			add_promise->set_value(-static_cast<int>(response.err));
	});
	std::string name_utf8(wxConvUTF8.cWC2MB(name.c_str()));
	client.add(cat, name_utf8);

	int res = add_future.get();
	if (res >= 0)
	{
		ret = static_cast<id_type>(res);
		return lwm_client::ERR_SUCCESS;
	}
	else
		return -res;
}

lwm_client::err_t del(lwm_client::category_t cat, id_type id)
{
	std::shared_ptr<std::promise<lwm_client::err_t>> del_promise = std::make_shared<std::promise<lwm_client::err_t>>();
	std::future<lwm_client::err_t> del_future = del_promise->get_future();
	client.set_callback([del_promise, cat](lwm_client::response response) {
		del_promise->set_value(response.err);
	});
	client.del(cat, id);

	return del_future.get();
}

void group::submit()
{
	if (gID < 0)
		return;
	std::string data;
	std::string name_utf8(wxConvUTF8.cWC2MB(name.c_str()));
	uint32_t name_len = boost::endian::native_to_little(static_cast<uint32_t>(name_utf8.size()));
	data.append(reinterpret_cast<char*>(&name_len), sizeof(uint32_t));
	data.append(name_utf8);

	uint16_t user_count = boost::endian::native_to_little(static_cast<uint16_t>(members.size()));
	data.append(reinterpret_cast<char*>(&user_count), sizeof(uint16_t));
	for (id_type ID : members)
		data.append(reinterpret_cast<char*>(&ID), sizeof(id_type));

	client.modify(lwm_client::CAT_GROUP, static_cast<id_type>(gID), data);
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
	toSingleLine(str_utf8);
	str_len = boost::endian::native_to_little(static_cast<uint32_t>(str_utf8.size()));
	data.append(reinterpret_cast<char*>(&str_len), sizeof(uint32_t));
	data.append(str_utf8);

	uint16_t group_count = boost::endian::native_to_little(static_cast<uint16_t>(groups.size()));
	data.append(reinterpret_cast<char*>(&group_count), sizeof(uint16_t));
	for (id_type_l ID : groups)
	{
		if (ID >= 0)
		{
			id_type ID_send = static_cast<id_type>(ID);
			data.append(reinterpret_cast<char*>(&ID_send), sizeof(id_type));
		}
	}

	uint16_t work_count = boost::endian::native_to_little(static_cast<uint16_t>(works.size()));
	data.append(reinterpret_cast<char*>(&work_count), sizeof(uint16_t));
	for (id_type_l ID : works)
	{
		if (ID >= 0)
		{
			id_type ID_send = static_cast<id_type>(ID);
			data.append(reinterpret_cast<char*>(&ID_send), sizeof(id_type));
		}
	}

	client.modify(lwm_client::CAT_MEMBER, uID, data);
}

void work::submit()
{
	if (wID < 0)
		return;
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

	client.modify(lwm_client::CAT_WORK, static_cast<id_type>(wID), data);
}

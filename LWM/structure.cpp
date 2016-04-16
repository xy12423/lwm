#include "stdafx.h"
#include "structure.h"

extern lwm_client client;

grpListTp grpList;
memListTp memList;
workListTp workList;

void group::submit()
{
	std::string data;
	std::string name_utf8 = wxConvUTF8.cWC2MB(name.c_str());
	uint32_t name_len = boost::endian::native_to_little(static_cast<uint32_t>(name_utf8.size()));
	data.append(reinterpret_cast<char*>(&name_len), sizeof(uint32_t));
	data.append(name_utf8);

	uint16_t user_count = boost::endian::native_to_little(static_cast<uint16_t>(members.size()));
	data.append(reinterpret_cast<char*>(&user_count), sizeof(uint16_t));
	for (size_t uID : members)
		data.push_back(uID);

	client.modify(lwm_client::CAT_GROUP, gID, data);
}

void member::submit()
{
	std::string data;
	std::string str_utf8 = wxConvUTF8.cWC2MB(name.c_str());
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
	for (size_t gID : groups)
		data.push_back(gID);

	uint16_t work_count = boost::endian::native_to_little(static_cast<uint16_t>(works.size()));
	data.append(reinterpret_cast<char*>(&work_count), sizeof(uint16_t));
	for (size_t wID : works)
		data.push_back(wID);

	client.modify(lwm_client::CAT_MEMBER, uID, data);
}

void work::submit()
{
	std::string data;
	std::string str_utf8 = wxConvUTF8.cWC2MB(name.c_str());
	uint32_t str_len = boost::endian::native_to_little(static_cast<uint32_t>(str_utf8.size()));
	data.append(reinterpret_cast<char*>(&str_len), sizeof(uint32_t));
	data.append(str_utf8);

	str_utf8 = wxConvUTF8.cWC2MB(info.c_str());
	str_len = boost::endian::native_to_little(static_cast<uint32_t>(str_utf8.size()));
	data.append(reinterpret_cast<char*>(&str_len), sizeof(uint32_t));
	data.append(str_utf8);

	uint16_t user_count = boost::endian::native_to_little(static_cast<uint16_t>(members.size()));
	data.append(reinterpret_cast<char*>(&user_count), sizeof(uint16_t));
	for (size_t uID : members)
		data.push_back(uID);

	client.modify(lwm_client::CAT_WORK, wID, data);
}

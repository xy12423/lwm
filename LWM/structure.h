#pragma once

#ifndef _H_STRUCTURE
#define _H_STRUCTURE

#include "lwm_client.h"

typedef lwm_client::id_type id_type;
typedef int32_t id_type_l;
typedef lwm_client::data_size_type data_size_type;

class group
{
public:
	group(id_type_l _gID, const std::wstring &_name)
		:gID(_gID), name(_name) {}
	group(id_type_l _gID, std::wstring &&_name)
		:gID(_gID), name(std::move(_name)) {}
	id_type_l getGID() const { return gID; };
	const std::wstring &getName() const { return name; };
	void getMember(std::list<id_type> &ret) const { for (id_type uID : members) { ret.push_back(uID); }; };

	void editName(const std::wstring &newName) { name = newName; submit(); };
	void addMember(id_type uID) { members.insert(uID); submit(); };
	void delMember(id_type uID) { members.erase(uID); submit(); };

	friend void load_group(id_type id, data_view& data);
private:
	void submit();

	id_type_l gID;
	std::wstring name;
	std::set<id_type> members;
};

typedef std::map<id_type_l, group> grpListTp;
extern grpListTp grpList;

struct uExtInfo
{
	enum status_tp
	{
		ST_AVAILABLE,
		ST_BUSY,
		ST_UNAVAILABLE
	};

	uExtInfo() {}
	uExtInfo(const std::wstring &_src, const std::wstring &_info, status_tp _status)
		:src(_src), info(_info), status(_status) {}
	uExtInfo(std::wstring &&_src, std::wstring &&_info, status_tp _status)
		:src(std::move(_src)), info(std::move(_info)), status(_status) {}

	std::wstring src;
	std::wstring info;
	status_tp status;
};

class member
{
public:
	member(id_type _uID, const std::wstring &_name, const uExtInfo &_extInfo)
		:uID(_uID), name(_name), extInfo(_extInfo) {}
	member(id_type _uID, const std::wstring &_name, uExtInfo &&_extInfo)
		:uID(_uID), name(_name), extInfo(std::move(_extInfo)) {}
	member(id_type _uID, std::wstring &&_name, uExtInfo &&_extInfo)
		:uID(_uID), name(std::move(_name)), extInfo(std::move(_extInfo)) {}
	id_type getUID() const { return uID; };
	const std::wstring &getName() const { return name; };
	const uExtInfo &getExtInfo() const { return extInfo; };
	void getGroup(std::list<id_type_l> &ret) const { for (id_type_l gID : groups) { ret.push_back(gID); }; };
	bool isInGroup(id_type_l gID) const { return groups.find(gID) != groups.end(); }
	size_t getGroupCount() const { return groups.size(); };
	void getWork(std::list<id_type_l> &ret) const { for (id_type_l wID : works) { ret.push_back(wID); }; };
	bool isInWork(id_type_l wID) const { return works.find(wID) != works.end(); }
	size_t getWorkCount() const { return works.size(); };

	void editName(const std::wstring &newName) { name = newName; };
	void editName(std::wstring &&newName) { name = std::move(newName); };
	void editInfo(const uExtInfo &newInfo) { extInfo = newInfo; };
	void editInfo(uExtInfo &&newInfo) { extInfo = std::move(newInfo); };
	void addGroup(id_type_l gID) { groups.insert(gID); if (gID >= 0) submit(); };
	void delGroup(id_type_l gID) { groups.erase(gID); if (gID >= 0) submit(); };
	void addWork(id_type_l wID) { works.insert(wID); if (wID >= 0) submit(); };
	void delWork(id_type_l wID) { works.erase(wID); if (wID >= 0) submit();
	};

	void submit();

	friend void load_member(id_type id, data_view& data);
private:
	id_type uID;
	std::wstring name;
	uExtInfo extInfo;
	std::set<id_type_l> groups, works;
};

typedef std::map<id_type, member> memListTp;
extern memListTp memList;

class work
{
public:
	work(id_type_l _wID, const std::wstring &_name, const std::wstring &_info)
		:wID(_wID), name(_name), info(_info) {}
	work(id_type_l _wID, std::wstring &&_name, std::wstring &&_info)
		:wID(_wID), name(std::move(_name)), info(std::move(_info)) {}
	id_type_l getWID() const { return wID; };
	const std::wstring &getName() const { return name; };
	const std::wstring &getInfo() const { return info; };
	void getMember(std::list<id_type> &ret) const { for (id_type uID : members) { ret.push_back(uID); }; };

	void editName(const std::wstring &newName) { name = newName; };
	void editName(std::wstring &&newName) { name = std::move(newName); };
	void editInfo(const std::wstring &newInfo) { info = newInfo; };
	void editInfo(std::wstring &&newInfo) { info = std::move(newInfo); };
	void addMember(id_type uID) { members.insert(uID); submit(); };
	void delMember(id_type uID) { members.erase(uID); submit(); };

	void submit();

	friend void load_work(id_type id, data_view& data);
private:
	id_type_l wID;
	std::wstring name;
	std::wstring info;
	std::set<id_type> members;
};

typedef std::map<id_type_l, work> workListTp;
extern workListTp workList;

void load_group(id_type id, data_view& data);
void load_work(id_type id, data_view& data);
void load_member(id_type id, data_view& data);
lwm_client::err_t list(lwm_client::category_t cat);
lwm_client::err_t add(lwm_client::category_t cat, const std::wstring& name, id_type& ret);
lwm_client::err_t del(lwm_client::category_t cat, id_type id);

const id_type_l default_id = -1;

#endif

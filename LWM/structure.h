#pragma once

#ifndef _H_STRUCTURE
#define _H_STRUCTURE

#include "lwm_client.h"

class group
{
public:
	group(size_t _gID, const std::wstring &_name)
		:gID(_gID), name(_name) {}
	group(size_t _gID, std::wstring &&_name)
		:gID(_gID), name(_name) {}
	size_t getGID() { return gID; };
	const std::wstring &getName() const { return name; };
	void getMember(std::list<size_t> &ret) const { for (size_t uID : members) { ret.push_back(uID); }; };

	void editName(const std::wstring &newName) { name = newName; submit(); };
	void addMember(size_t uID) { members.emplace(uID); submit(); };
	void delMember(size_t uID) { members.erase(uID); submit(); };
private:
	void submit();

	size_t gID;
	std::wstring name;
	std::set<size_t> members;
};

typedef std::map<size_t, group*> grpListTp;
extern grpListTp grpList;

struct uExtInfo
{
	uExtInfo() {}
	uExtInfo(const std::wstring &_src, const std::wstring &_info)
		:src(_src), info(_info) {}
	uExtInfo(std::wstring &&_src, std::wstring &&_info)
		:src(_src), info(_info) {}

	std::wstring src;
	std::wstring info;
};

class member
{
public:
	member(size_t _uID, const std::wstring &_name, const uExtInfo &_extInfo)
		:uID(_uID), name(_name), extInfo(_extInfo) {}
	member(size_t _uID, std::wstring &&_name, uExtInfo &&_extInfo)
		:uID(_uID), name(_name), extInfo(_extInfo) {}
	size_t getUID() const { return uID; };
	const std::wstring &getName() const { return name; };
	const uExtInfo &getExtInfo() const { return extInfo; };
	void getGroup(std::list<size_t> &ret) const { for (size_t gID : groups) { ret.push_back(gID); }; };
	size_t getGroupCount() const { return groups.size(); };
	bool isInGroup(size_t gID) const { return groups.find(gID) != groups.end(); }
	void getWork(std::list<size_t> &ret) const { for (size_t wID : works) { ret.push_back(wID); }; };

	void editName(const std::wstring &newName) { name = newName; };
	void editName(std::wstring &&newName) { name = std::move(newName); };
	void editInfo(const uExtInfo &newInfo) { extInfo = newInfo; };
	void editInfo(uExtInfo &&newInfo) { extInfo = std::move(newInfo); };
	void addGroup(size_t gID) { groups.emplace(gID); submit(); };
	void delGroup(size_t gID) { groups.erase(gID); submit(); };
	void addWork(size_t wID) { works.emplace(wID); submit(); };
	void delWork(size_t wID) { works.erase(wID); submit(); };

	void submit();
private:
	size_t uID;
	std::wstring name;
	uExtInfo extInfo;
	std::set<size_t> groups, works;
};

typedef std::map<size_t, member*> memListTp;
extern memListTp memList;

class work
{
public:
	work(size_t _wID, const std::wstring &_name, const std::wstring &_info)
		:wID(_wID), name(_name), info(_info) {}
	work(size_t _wID, std::wstring &&_name, std::wstring &&_info)
		:wID(_wID), name(_name), info(_info) {}
	size_t getWID() const { return wID; };
	const std::wstring &getName() const { return name; };
	const std::wstring &getInfo() const { return info; };
	void getMember(std::list<size_t> &ret) const { for (size_t uID : members) { ret.push_back(uID); }; };

	void editName(const std::wstring &newName) { name = newName; };
	void editName(std::wstring &&newName) { name = std::move(newName); };
	void editInfo(const std::wstring &newInfo) { info = newInfo; };
	void editInfo(std::wstring &&newInfo) { info = std::move(newInfo); };
	void addMember(size_t uID) { members.emplace(uID); submit(); };
	void delMember(size_t uID) { members.erase(uID); submit(); };

	void submit();
private:
	size_t wID;
	std::wstring name;
	std::wstring info;
	std::set<size_t> members;
};

typedef std::map<size_t, work*> workListTp;
extern workListTp workList;

#endif

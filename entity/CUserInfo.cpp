//
// Created by zielin on 01.11.23.
//

#include "CUserInfo.h"
#include <Poco/StringTokenizer.h>

using namespace Poco;

CUserInfo::CUserInfo(const std::string& name,
	long uid,
	long gid,
	const std::string& homeDir,
	const std::string& shell,
	const std::string& desc)
{
	insert("name", name);
	insert("uid", uid);
	insert("gid", gid);
	insert("home", homeDir);
	insert("shell", shell);
	insert("description", desc);
}

std::string CUserInfo::getHome() const
{
	return getVar("home");
}

std::string CUserInfo::getName() const
{
	return getVar("name");
}

long CUserInfo::getUID() const
{
	return getVar("uid");
}

long CUserInfo::getGID() const
{
	return getVar("gid");
}

std::string CUserInfo::getShell() const
{
	return getVar("shell");
}

std::string CUserInfo::getDescription() const
{
	return getVar("description");
}

CUserInfo::Ptr CUserInfo::fromPasswd(const std::string& passwdEntry)
{
	StringTokenizer st(passwdEntry, ":", StringTokenizer::Options::TOK_TRIM);
	return new CUserInfo(st[0], std::stoul(st[2]), std::stoul(st[3]), st[5], st[6], st[4]);
}

CUserInfo::CUserInfo()
{

}

//
// Created by zielin on 01.11.23.
//

#include "CGroupInfo.h"
#include <Poco/StringTokenizer.h>

using namespace Poco;

CGroupInfo::CGroupInfo(const std::string& name, uint gid, const std::vector<std::string>& members)
{
	insert("name", name);
	insert("gid", gid);
	insert("members", members);
}

std::string CGroupInfo::getName() const
{
	return getVar("name");
}

uint CGroupInfo::getGID() const
{
	return getVar("gid", 0);
}

std::vector<std::string> CGroupInfo::getMembers() const
{
	return getVar("members").extract<std::vector<std::string>>();
}

CGroupInfo::Ptr CGroupInfo::fromGroupEntry(const std::string& line)
{
	if (line.empty())
	{
		return nullptr;
	}

	StringTokenizer st(line, ":", StringTokenizer::Options::TOK_TRIM);
	std::vector<std::string> members;

	if (st.count() == 4)
	{
		StringTokenizer
			st1(st[3], ",", StringTokenizer::Options::TOK_TRIM | StringTokenizer::Options::TOK_IGNORE_EMPTY);

		for (auto i = 0; i < st1.count(); ++i)
		{
			members.push_back(st1[i]);
		}
	}

	return new CGroupInfo(st[0], std::stoul(st[2]), members);
}

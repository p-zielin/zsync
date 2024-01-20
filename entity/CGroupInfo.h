//
// Created by zielin on 01.11.23.
//

#ifndef STORAGE_CGROUPINFO_H
#define STORAGE_CGROUPINFO_H

#include "CBaseEntity.h"

class CGroupInfo : public CBaseEntity
{
public:
	typedef Poco::SharedPtr<CGroupInfo> Ptr;
	CGroupInfo(const std::string& name, uint gid, const std::vector<std::string>& members);

	static CGroupInfo::Ptr fromGroupEntry(const std::string& line);

	std::string getName() const;
	uint getGID() const;

	std::vector<std::string> getMembers() const;
};


#endif //STORAGE_CGROUPINFO_H

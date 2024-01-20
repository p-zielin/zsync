//
// Created by zielin on 01.11.23.
//

#ifndef STORAGE_CUSERINFO_H
#define STORAGE_CUSERINFO_H

#include "CBaseEntity.h"

class CUserInfo : public CBaseEntity
{
public:
	typedef Poco::SharedPtr<CUserInfo> Ptr;

	CUserInfo();
	CUserInfo(const std::string& name,
		long uid,
		long gid,
		const std::string& homeDir,
		const std::string& shell,
		const std::string& desc = "");

	static CUserInfo::Ptr fromPasswd(const std::string& passwdEntry);

	[[nodiscard]] std::string getHome() const;
	[[nodiscard]] std::string getName() const;
	[[nodiscard]] long getUID() const;
	[[nodiscard]] long getGID() const;

	[[nodiscard]] std::string getShell() const;
	[[nodiscard]] std::string getDescription() const;
};


#endif //STORAGE_CUSERINFO_H

#include "CVirtualUser.h"

namespace entity
{

	CVirtualUser::CVirtualUser()
	= default;

	std::string CVirtualUser::getName() const
	{
		return getVar("name");
	}

	std::string CVirtualUser::getMboxDir() const
	{
		return getVar("mboxDir");
	}

	CUserService::Ptr CVirtualUser::getUserService() const
	{
		return m_service;
	}

	void CVirtualUser::setUserService(const CUserService::Ptr & us)
	{
		m_service = us;
	}

	void CVirtualUser::setChmod(int i)
	{
		m_chmod = i;
	}

	int CVirtualUser::getChmod() const
	{
		return m_chmod;
	}

	unsigned long CVirtualUser::getQUotaLimitBytes() const
	{
		if (!contains("quotaLimit"))
		{
			return 0;
		}

		return getVar("quotaLimit").extract<Poco::JSON::Object::Ptr>()->getValue<unsigned long>("value");
	}

	bool CVirtualUser::isResponderEnabled() const
	{
		if (!getVar("responderEnabled", false))
		{
			return false;
		}

		return true;
	}

	std::string CVirtualUser::getResponderBody() const
	{
		return getVar("responderBody", "").extract<std::string>();
	}

	std::string CVirtualUser::getResponderSubject() const
	{
		return getVar("responderSubject", "").extract<std::string>();
	}

	bool CVirtualUser::isRedirectEnabled() const
	{
		return getVar("redirectEnabled", false);
	}

	bool CVirtualUser::isLeaveCopyEnabled() const
	{
		return getVar("leaveCopy", true);
	}

	bool CVirtualUser::isDeleteSpam() const
	{
		return getVar("deleteSpam", false);
	}

	bool CVirtualUser::isAllIntoMailbox() const
	{
		return getVar("allIntoMailbox", false);
	}

	int CVirtualUser::getUID() const
	{
		return getVar("uid", 0);
	}

	bool CVirtualUser::isSnapshot() const
	{
		return getVar("snapshots", true);
	}

	void CVirtualUser::fromJSON(const JSON::Object::Ptr& obj)
	{
		CBaseEntity::fromJSON(obj);

		if (obj->has("userService"))
		{
			m_service = new entity::CUserService();
			m_service->fromJSON(obj->getObject("userService"));
		}
	}

	std::string CVirtualUser::getSnapshotName() const
	{
		return getVar("snapshot", "");
	}
}

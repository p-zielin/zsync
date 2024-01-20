#include "CStorageInfo.h"

namespace entity
{

	void CStorageInfo::setDatasetInfo(CDatasetInfo::Ptr dataset)
	{
		insert("mountpoint", dataset->getMountPoint());
//		insert("totalSpace", dataset->getUsage() + dataset->getAvail());
		insert("usedSpace", dataset->getUsage());
	}

	unsigned long CStorageInfo::getId() const
	{
		return getVar("id");
	}

	std::string CStorageInfo::getName() const
	{
		return getVar("name");
	}

	bool CStorageInfo::getOnlyLocal() const
	{
		return getVar("onlyLocal");
	}

	std::string CStorageInfo::getRestUrl() const
	{
		return getVar("restUrl");
	}

	std::string CStorageInfo::getMountPath() const
	{
		return getVar("mountpath");
	}

	void entity::CStorageInfo::setMountPath(const std::string& path)
	{
		erase("mountpath");
		insert("mountpath", path);
	}

	int CStorageInfo::getTotalSpace() const
	{
		return getVar("totalSpace");
	}

	int CStorageInfo::getUsedSpace() const
	{
		return getVar("usedSpace");
	}

	CStorageInfo::CStorageInfo()
	{

	}

	void CStorageInfo::setRestUrl(const std::string& url)
	{
		insert("restUrl", url);
	}

	void CStorageInfo::setId(unsigned long id)
	{
		insert("id", id);
	}

	void CStorageInfo::setName(const std::string& name)
	{
		insert("name", name);
	}

	std::string CStorageInfo::getDataset() const
	{
		return getVar("dataset");
	}

	void CStorageInfo::setDataset(const std::string& dataset)
	{
		insert("dataset", dataset);
	}

	int entity::CStorageInfo::getJailGID() const
	{
		return getVar("jailGID", 0);
	}

	void CStorageInfo::setMailGID(uint gid)
	{
		erase("mailGID");
		insert("mailGID", gid);
	}

	uint CStorageInfo::getMailGID() const
	{
		return getVar("mailGID", 0);
	}

	int CStorageInfo::getMailChmod() const
	{
		return m_mailChmod;
	}

	void CStorageInfo::setMailChmod(int newMailChmod)
	{
		m_mailChmod = newMailChmod;
	}

	int CStorageInfo::getJailChmod() const
	{
		return m_jailChmod;
	}

	void CStorageInfo::setJailChmod(int jailChmod)
	{
		m_jailChmod = jailChmod;
	}

	int CStorageInfo::getParentChmod() const
	{
		return m_parentChmod;
	}

	void CStorageInfo::setParentChmod(int newParentChmod)
	{
		m_parentChmod = newParentChmod;
	}

	void CStorageInfo::setNetworkAccess(const std::string& address)
	{
		erase("networkAccess");
		insert("networkAccess", address);
	}

	std::string CStorageInfo::getNetworkAccess() const
	{
		return getVar("networkAccess");
	}

	void entity::CStorageInfo::setJailGID(uint gid)
	{
		erase("jailGID");
		insert("jailGID", gid);
	}

	int CStorageInfo::getWwwUID() const
	{
		return m_wwwUID;
	}

	void CStorageInfo::setWwwUID(int newWwwUID)
	{
		m_wwwUID = newWwwUID;
	}

}

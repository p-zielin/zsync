//
// Created by zielin on 01.11.23.
//

#include "../subsystem/CHelper.h"

#include "CDatasetInfo.h"
#include <Poco/File.h>
#include <Poco/StringTokenizer.h>
#include <Poco/JSON/Array.h>
#include <Poco/Logger.h>
#include <utility>

using namespace Poco;

namespace entity
{

	uint CDatasetInfo::getUID() const
	{
		return getVar("uid", 0);
	}

	std::string CDatasetInfo::getName() const
	{
		return getVar("name");
	}

	void CDatasetInfo::setName(const std::string& name)
	{
		erase("name");
		insert("name", name);
	}

    CDatasetInfo::CDatasetInfo(const std::string& path)
	{
		setName(path);
	}

	CDatasetInfo::Ptr CDatasetInfo::fromZfsLine(const std::string& zfsLine)
	{
        StringTokenizer st(zfsLine, "\t");
        CDatasetInfo::Ptr ds = new CDatasetInfo(st[0]);

		UInt64 tmp = 0;

		if (NumberParser::tryParseUnsigned64(st[1], tmp))
		{
			ds->m_quota.addBytes(tmp);
		}
		
		if (NumberParser::tryParseUnsigned64(st[2], tmp))
		{
			ds->m_refquota.addBytes(tmp);
		}

		if (NumberParser::tryParseUnsigned64(st[3], tmp))
		{
			ds->m_used.addBytes(tmp);
		}

		ds->setMountPoint(st[5]);
        ds->setUID(CHelper::getUID(ds->getMountPoint()));
        ds->m_avail.addBytes(std::stoull(st[6]));

		if (st.count() >= 9)
		{
			ds->setCreationTime(Timestamp(std::stol(st[8]) * 1000000));
		}

        return ds;
    }

    void CDatasetInfo::setRelativeName(const std::string &parentName)
    {
        auto name = getName();
		erase("relativeName");

		if (parentName.size() + 1 < name.size())
        {
			insert("relativeName",  name.substr(parentName.size() + 1));
        }
    }

	CDatasetInfo::Ptr CDatasetInfo::createChildren(const std::string& name) const
	{
        auto newName = getName() + (name.empty() ? "" : "/" + name);
        auto ret = new CDatasetInfo(*this);
        ret->setName(newName);
		ret->setMountPoint(getMountPoint() + (name.empty() ? "" : "/" + name));
        return ret;
	}

    std::vector<entity::CSnapshotInfo::Ptr> entity::CDatasetInfo::getSnapshots() const
    {
        return m_snapshots;
	}

    void CDatasetInfo::setSnapshots(const std::vector<entity::CSnapshotInfo::Ptr> & snaps)
    {
        m_snapshots = snaps;
    }

    void entity::CDatasetInfo::appendSnapshot(const entity::CSnapshotInfo::Ptr &snapshot)
    {
        m_snapshots.emplace_back(snapshot);
    }

	CByteCounter CDatasetInfo::getUsage() const
	{
		return m_used;
	}

	CByteCounter CDatasetInfo::getQuota() const
	{
		return m_quota;
	}

	CByteCounter CDatasetInfo::getRefQuota() const
	{
		return m_refquota;
	}

	std::string CDatasetInfo::getMountPoint() const
	{
		return getVar("mountPoint", "");
	}

	void entity::CDatasetInfo::setQuota(unsigned long long quota)
	{
		m_quota.reset();
		m_quota.addBytes(quota);
	}

	CByteCounter CDatasetInfo::getAvail() const
	{
		return m_avail;
	}

	void CDatasetInfo::setUID(uint uid)
	{
		erase("uid");
		insert("uid", uid);
	}

	void CDatasetInfo::setGID(uint gid)
	{
		erase("gid");
		insert("gid", gid);
	}

	uint CDatasetInfo::getGID() const
	{
		return getVar("gid", 0);
	}

	std::string CDatasetInfo::getShareNFS() const
	{
		return getVar("shareNFS", "");
	}

	void CDatasetInfo::setShareNFS(const std::string& sharenfs)
    {
        erase("shareNFS");
		insert("shareNFS", sharenfs);
    }

	uint CDatasetInfo::getChmod() const
	{
		return getVar("chmod", 755);
	}

	void CDatasetInfo::setChmod(uint chmod)
	{
		erase("chmod");
		insert("chmod", chmod);
	}

	void CDatasetInfo::setMountPoint(const std::string& newMountPoint)
	{
		erase("mountPoint");
		insert("mountPoint", newMountPoint);
	}

	std::string CDatasetInfo::getRelativeName() const
    {
        return getVar("relativeName", "");
    }

	Timestamp CDatasetInfo::getCreationTime() const
	{
		return getVar("creationTime", 0);
	}

	CDatasetInfo::CDatasetInfo(const CDatasetInfo& di)
	 : CBaseEntity(di) {
		*this = di;
	}

	JSON::Object::Ptr CDatasetInfo::toJSON() const
	{
		JSON::Object::Ptr obj = CBaseEntity::toJSON();

		if (!m_snapshots.empty())
		{
			JSON::Array::Ptr arr = new JSON::Array;
			for (auto & s : m_snapshots)
			{
				arr->add(s->toJson());
            }
            obj->set("snapshots", arr);
        }

		obj->set("used", m_used.getKBytes());
		obj->set("quota", m_quota.getKBytes());
		obj->set("avail", m_avail.getKBytes());
		obj->set("ref_quota", m_refquota.getKBytes());
		return obj;
	}

	CDatasetInfo::CDatasetInfo(const JSON::Object::Ptr& json)
	{
		if (json.isNull())
		{
			return;
		}

		CBaseEntity::fromJSON(json);
		m_used.addKBytes(json->getValue<UInt64>("used"));
		m_quota.addKBytes(json->getValue<UInt64>("quota"));
		m_avail.addKBytes(json->getValue<UInt64>("avail"));
		m_refquota.addKBytes(json->getValue<UInt64>("ref_quota"));
	}

	void CDatasetInfo::setCreationTime(const Timestamp& creationTime)
	{
		erase("creationTime");
		insert("creationTime", creationTime);
	}

	std::vector<std::string> CDatasetInfo::getFirstAvailableSnapshot(const std::string& backupPrefix) const
	{
		std::vector<std::string> ret;

		if (backupPrefix.empty())
		{
			return ret;
		}

		auto iter = m_snapshots.rbegin();

		while (iter != m_snapshots.rend())
		{
			if ( (*iter)->getName().rfind(backupPrefix) == 0)
			{
				return ret;
			}

			ret.emplace_back((*iter)->getName());
			++iter;
		}

		return ret;
	}

	void CDatasetInfo::setQuota(const CByteCounter& newQuota)
	{
		m_quota = newQuota;
	}
}

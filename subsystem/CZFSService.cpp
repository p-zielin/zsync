//
// Created by zielin on 01.11.23.
//

#include "CZFSService.h"
#include "CShellSubsystem.h"

#include <Poco/Logger.h>
#include <Poco/Timestamp.h>
#include <Poco/StringTokenizer.h>
#include <Poco/Util/Application.h>
#include <Poco/Pipe.h>
#include <unistd.h>
#include <sys/stat.h>

using namespace Poco;

namespace service
{
const std::string CZFSService::m_zfsColumns = "name,quota,refquota,used,type,mountpoint,avail,written,creation";

	entity::CDatasetInfo::Ptr CZFSService::listDataset(const std::string& name, bool includeSnapshots)
	{
		CShellSubsystem::check(name);
		Process::Args args;
		args.emplace_back("list");
		args.emplace_back("-H");
        args.emplace_back("-p");
		args.emplace_back("-o");
		args.emplace_back(m_zfsColumns);
		args.emplace_back(name);

		auto lines = CShellSubsystem::getOutputLines("zfs", args);

		if (lines.empty())
		{
			return nullptr;
		}

		auto ds = entity::CDatasetInfo::fromZfsLine(lines[0]);
        if (!m_rootDataset.isNull())
        {
            ds->setRelativeName(m_rootDataset->getName());
        }
		getStat(*ds);

		if (includeSnapshots)
		{
			attachSnapshots(ds);
		}

		return ds;
	}

	void CZFSService::attachSnapshots(entity::CDatasetInfo::Ptr& ds, const std::string & prefixFilter)
	{
		Process::Args args;
		args.emplace_back("list");
		args.emplace_back("-H");
        args.emplace_back("-p");
		args.emplace_back("-o");
        args.emplace_back("name,creation");
		args.emplace_back("-t");
		args.emplace_back("snap");
		args.emplace_back("-d");
		args.emplace_back("1");
		args.emplace_back("-r");
		args.emplace_back(ds->getName());
        std::vector<entity::CSnapshotInfo::Ptr> snaps;

        for (const auto & snap : CShellSubsystem::getOutputLines("zfs", args))
        {
            StringTokenizer st(snap, "\t");

            if (st.count() != 2)
            {
                continue;
            }

            auto name = st[0];
            const auto &found = name.find("@" + prefixFilter, 0);

            if (found == std::string::npos)
            {
                continue;
            }

            auto snapshot = new entity::CSnapshotInfo();
            snapshot->setName(name.substr(found + 1));
            snapshot->setCreationTime(std::stoul(st[1]) * 1000000);
            snaps.emplace_back(snapshot);
        }

        ds->setSnapshots(snaps);
    }

	bool CZFSService::removeSnapshots(const entity::CDatasetInfo& dataset,
		const std::string& from,
        const std::string& to,
        bool recursive)
	{
		auto range = dataset.getName() + "@" + (from != "%" ? from + "," : from) + to;
		Process::Args args;
        args.emplace_back("zfs");
		args.emplace_back("destroy");
        if (recursive)
        {
            args.emplace_back("-r");
        }
		args.emplace_back(range);

        Logger::root().debug("sudo " + cat<std::string>(" ", args.cbegin(), args.cend()));
        return Process::launch("sudo", args).wait() == 0;
    }

    bool CZFSService::removeSnapshot(const entity::CDatasetInfo &dataset, const std::string &name, bool recursive)
    {
        Process::Args args;
        args.emplace_back("zfs");
        args.emplace_back("destroy");

        if (recursive)
        {
            args.emplace_back("-r");
        }

        args.emplace_back(dataset.getName() + "@" + name);
        Logger::root().debug("sudo " + cat<std::string>(" ", args.cbegin(), args.cend()));
        return Process::launch("sudo", args).wait() == 0;
    }

	std::vector<entity::CDatasetInfo::Ptr> CZFSService::listDatasets(const entity::CDatasetInfo& rootDataset, bool includeSnapshots, bool recursive, int level)
	{
        Logger::root().debug("Listowanie datasetu '%s'", rootDataset.getName());

		Process::Args args;
		args.emplace_back("list");
		args.emplace_back("-H");
        args.emplace_back("-p");
		args.emplace_back("-o");
		args.emplace_back(m_zfsColumns);

        if (level > 0)
        {
            args.emplace_back("-d");
            args.emplace_back(std::to_string(level));
        }

        if (recursive)
        {
            args.emplace_back("-r");
        }

		args.emplace_back(rootDataset.getName());

		std::vector<entity::CDatasetInfo::Ptr> ret;
		std::vector<std::string> snaps;

		auto datasetList = CShellSubsystem::getOutputLines("zfs", args);

		// Na pierwszym miejscu jest nasz rootDataset i nie chcemy go.
		if (!datasetList.empty())
		{
			datasetList.erase(datasetList.begin());
		}

		for (const std::string& line : datasetList)
		{
			auto ds = entity::CDatasetInfo::fromZfsLine(line);
			if (!m_rootDataset.isNull())
			{
				ds->setRelativeName(m_rootDataset->getName());
			}

			getStat(*ds);

			if (includeSnapshots)
			{
				attachSnapshots(ds);
			}

            ret.emplace_back(ds);
        }

		return ret;
	}

	bool CZFSService::createDatasetByName(const std::string& name)
	{
		Process::Args args;
		args.emplace_back("zfs");
		args.emplace_back("create");
		args.emplace_back(name);

		Logger::root().debug("sudo zfs create " + name);

		if (Process::launch("sudo", args).wait() != 0)
		{
			return false;
		}

		return true;
	}

	void CZFSService::printLastError()
	{
		char error[1024];
		memset(error, 0, 1024);
		perror(error);

        if (strlen(error) > 0)
        {
            Logger::root().error("printLastError:" + std::string(error));
        }
        else
        {
            Logger::root().debug("Brak informacji o błędzie");
        }
	}

	bool CZFSService::setQuota(const entity::CDatasetInfo::Ptr & ds)
	{
		CShellSubsystem::check(ds->getName());
		Process::Args args;
		args.emplace_back("zfs");
		args.emplace_back("set");

		if (ds->getQuota().getTotalMBytes() > 0)
		{
			args.emplace_back("quota=" + std::to_string(ds->getQuota().getTotalMBytes()) + "M");
		}
		else
		{
			args.emplace_back("quota=none");
		}

		args.emplace_back(ds->getName());
		Logger::root().debug("sudo " + cat<std::string>(" ", args.cbegin(), args.cend()));
		return Process::launch("sudo", args).wait() == 0;
	}

	bool CZFSService::createDataset(entity::CDatasetInfo::Ptr dataset)
	{
		CShellSubsystem::check(dataset->getName());

		auto found = listDataset(dataset->getName());

		if (!found.isNull())
		{
			Logger::root().warning("Istnieje już dataset '%s'", dataset->getName());
			return false;
		}

		if (!createDatasetByName(dataset->getName()))
		{
			return false;
		}

		return true;
	}

	bool service::CZFSService::destroyDataset(entity::CDatasetInfo::Ptr dataset)
	{
		if (m_rootDataset.isNull())
		{
			throw Poco::Exception("Usługa niezainicjowana");
		}

		if (dataset == m_rootDataset || dataset->getName() == m_rootDataset->getName())
		{
			throw Poco::Exception("Nie możesz usunąć datasetu głównego");
		}

		if (dataset->getName().rfind(m_rootDataset->getName(), 0) != 0)
		{
			throw Poco::Exception("Możesz usuwać jedynie datasety wewnątrz '%s'", m_rootDataset->getName());
		}

		CShellSubsystem::check(dataset->getName());
		Process::Args args;
		args.emplace_back("zfs");
		args.emplace_back("destroy");
		args.emplace_back("-r");
		args.emplace_back(dataset->getName());
		return Process::launch("sudo", args).wait() == 0;
	}

	void CZFSService::restoreACL(entity::CDatasetInfo::Ptr dataset)
	{
		if (chmod(dataset->getMountPoint().c_str(), dataset->getChmod()) != 0)
		{
			printLastError();
		}

		if (chown(dataset->getMountPoint().c_str(), dataset->getUID(), dataset->getGID()) != 0)
		{
			printLastError();
		}
	}

	bool CZFSService::restoreSettings(entity::CDatasetInfo::Ptr dataset)
	{
		Process::Args args;
		args.emplace_back("set");

		if (!dataset->getShareNFS().empty())
		{
			args.emplace_back("sharenfs=" + dataset->getShareNFS());
		}
		else
		{
			args.emplace_back("sharenfs=off");
		}

		if (dataset->getQuota().getTotalKBytes() > 0)
		{
			args.emplace_back("quota=" + std::to_string(dataset->getQuota().getTotalKBytes()) + "K");
		}
		else
		{
			args.emplace_back("quota=none");
		}

		if (args.size() == 1)
		{
			Logger::root().debug("Brak parametrów do ustawienia w datasecie '%s'", dataset->getName());
			return true;
		}

		args.emplace_back(dataset->getName());

		bool ret = true;

		if (Process::launch("zfs", args).wait() != 0)
		{
			ret = false;
			Logger::root().warning("Problem podczas przydzielania sharenfs,quota dla '%s'", dataset->getName());
		}

		// Osobno mountpoint, poniewaz moze sie wywalic, jesli dataset ma podrzedne datasety z ustawionym
		// mountpointem.
		Pipe pipe;
		Pipe pipe1;

		args.clear();
		args.emplace_back("set");
		args.emplace_back("mountpoint=" + dataset->getMountPoint());
		args.emplace_back(dataset->getName());
		return Process::launch("zfs", args, nullptr, &pipe, &pipe1).wait() == 0 && ret;
	}

	const char* CZFSService::name() const
	{
		return "ZFS";
	}

	void CZFSService::initialize(Util::Application& app)
	{
		Logger::root().debug("Inizjalizacja zfs");
        auto rootDataset = app.config().getString("storage.root_dataset");
        m_rootDataset = listDataset(rootDataset);

        if (m_rootDataset.isNull())
        {
            throw Poco::Exception("Nie ma datasetu głównego " + rootDataset);
        }
	}

	void CZFSService::uninitialize()
	{

	}

	void CZFSService::getStat(entity::CDatasetInfo& ds)
	{
		// Jeśli zamontowany, dopiszemy UID/GID
		if (!ds.getMountPoint().empty())
		{
			struct stat s{};
			stat(ds.getMountPoint().c_str(), &s);
			ds.setUID(s.st_uid);
			ds.setGID(s.st_gid);
		}
	}

	bool CZFSService::createSnapshot(const entity::CDatasetInfo& ds, const std::string& snapshot, bool recursive)
	{
		if (snapshot.empty() || ds.getName().empty())
		{
			return false;
		}

		Process::Args args;
		args.emplace_back("snap");

		if (recursive)
		{
			args.emplace_back("-r");
		}

        args.emplace_back(ds.getName() + "@" + snapshot);
        Logger::root().debug("zfs " + cat<std::string>(" ", args.cbegin(), args.cend()));
        return Process::launch("zfs", args).wait() == 0;
    }

    entity::CDatasetInfo::Ptr CZFSService::getRootDataset() const
    {
        return m_rootDataset;
    }

	bool CZFSService::rollback(const entity::CDatasetInfo::Ptr & ds, const std::string & snapshot)
	{
		Logger::root().notice("Rollback '%s' do '%s'", ds->getName(), snapshot);
		Process::Args args;
		args.emplace_back("rollback");
		args.emplace_back("-r");
		args.emplace_back(ds->getName() + "@" + snapshot);
        Logger::root().debug("zfs " + cat<std::string>(" ", args.cbegin(), args.cend()));
        return Process::launch("zfs", args).wait() == 0;
	}

	void CZFSService::setReadonly(const entity::CDatasetInfo& ds, bool ro)
	{
		Process::Args args;

		args.emplace_back("zfs");
		args.emplace_back("set");
		args.emplace_back("-o");
		args.emplace_back("readonly=" + std::string(ro ? "on" : "off"));
		args.push_back(ds.getName());
		Process::launch("sudo", args);
	}

}

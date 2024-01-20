//
// Created by zielin on 19.11.23.
//

#include "CCleanerWorker.h"

#include "subsystem/CZFSService.h"
#include "subsystem/backup/CBackupSubsystem.h"

#include <Poco/DateTimeFormatter.h>
#include <Poco/Logger.h>
#include <Poco/Util/Application.h>
#include <Poco/Mutex.h>

using namespace Poco;

Thread CCleanerWorker::m_thread;

CCleanerWorker::CCleanerWorker()
= default;

/*!
 * \brief Kasuje
 */
void CCleanerWorker::run()
{
	try
	{
		auto& app = Util::Application::instance();
		auto& backup = app.getSubsystem<service::CBackupSubsystem>();
		auto ts = app.config().getInt("backup.keep_days", 0);

		if (ts == 0)
		{
			Logger::root().notice("Not deleting overtimed snapshots because of backup.keep_days = 0");
			return;
		}

		Logger::root().notice("Delete overtimed snapshots (and datasets if I'm slave node).");

		// Ile dni dany snapshot może być przechowywany
        auto validity = Timespan(ts, 0, 0, 0, 0);
		auto& zfs = app.getSubsystem<service::CZFSService>();

		// Przejdź po datasetach
        std::vector<entity::CDatasetInfo::Ptr> datasets;

        // Master czyści snapshoty w datasecie glownym (rekurencyjnie)
        if (backup.getMode() == service::CBackupSubsystem::Mode::master)
        {
            datasets.emplace_back(zfs.listDataset(zfs.getRootDataset()->getName(), true));
        }
        // Czyszczenie kazdego datasetu osobno w trybie slave
        else
        {
            datasets = zfs.listDatasets(*zfs.getRootDataset(), true, true, 0);
        }

		for (const auto & ds : datasets)
		{
			// Jeśli dataset jest ignorowany i nie jest to dataset główny, to go pomiń.
			if (!backup.isDatasetAllowed(ds->getRelativeName()) && ds->getName() != zfs.getRootDataset()->getName())
			{
				Logger::root().debug("Omit '%s' because of filter regex", ds->getName());
				continue;
			}

			// Wszystkie snapshoty w datasecie
			auto snaps = ds->getSnapshots();
            auto iter = snaps.rbegin();

			// Liczba snapshotów pasujących do wzorca dla backupów. Zliczamy, bo nie chcemy kasować ostatniego snapshota z backupem.
			int backupCounter = backup.getMode() == service::CBackupSubsystem::Mode::master ? 2 : 1;

            // Idąc od najnowższych do najstarszych znajdź DRUGI snapshot, który jest przeterminowany
            while (iter != snaps.rend())
			{
                Logger::root().debug("Checking snapshot name and validity time in '%s@%s'", ds->getName(), (*iter)->getName());

				// Jeśli snapshot nie pasuje do wzorca lub nie jest przeterminowany, to szukaj dalej.
                if (backup.getMode() == service::CBackupSubsystem::Mode::master && (*iter)->getName().rfind(backup.getBackupPrefix()) != 0)
                {
                    Logger::root().debug("Omit snapshot because of name: '%s' not starts with '%s'", (*iter)->getName(), backup.getBackupPrefix());
					++iter;
					continue;
				}

				if (((*iter)->getCreationTime() + validity) > DateTime().timestamp())
				{
					Logger::root().debug("Omit snapshot because of timestamp");
					++iter;
					continue;
				}

				// Ostatni snapshot, nawet jeśli przeterminowany zawsze trzymamy. W przypadku długotrwałych transferów, snapshoty
				// mogą ulec przeterminowaniu, ale musimy mieć ostatni punkt zaczepienia na wypadek ponowienia transferu itp.
				if (--backupCounter > 0)
				{
                    Logger::root().debug("Omit snapshot because of counter");
					++iter;
					continue;
				}

				// Dopiero drugi przeterminowany snapshot, pasujący do wzorca  możemy skasować
				Logger::root().notice("Delete everything in '%s' until '%s' (%s) inclusive.", ds->getName(), (*iter)->getName(), DateTimeFormatter::format((*iter)->getCreationTime(), "%Y-%m-%d %H:%M:%S"));
				service::CZFSService::removeSnapshots(*ds, "%", (*iter)->getName(), true);

				// W trybue slave usuwaj również te datasety, które po usunięciu snapshotów mają ich liczbę = 0.
				if (backup.getMode() == service::CBackupSubsystem::Mode::slave && app.config().getBool("backup.remove_datasets", false))
				{
					Logger::root().notice("Delete (unimplemented) dataset '%s'", ds->getName());
				}

				break;
			}
		}
	}
	catch (const Poco::Exception & ex)
	{
		Logger::root().error(ex.displayText());
	}
}

CCleanerWorker *CCleanerWorker::instance()
{
    static FastMutex mtx;
    FastMutex::ScopedLock lock(mtx);
    static CCleanerWorker* instance = new CCleanerWorker;
    return instance;
}

bool CCleanerWorker::start()
{
    if (!m_thread.isRunning())
    {
        m_thread.start(*this);
        return true;
    }

    return false;
}

void CCleanerWorker::join()
{
    m_thread.join();
}

bool CCleanerWorker::isRunning() const
{
    return m_thread.isRunning();
}

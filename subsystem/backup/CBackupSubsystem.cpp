//
// Created by zielin on 13.11.23.
//

#include "CBackupSubsystem.h"

#include <Poco/Net/ServerSocket.h>
#include <Poco/Util/Application.h>
#include <Poco/StringTokenizer.h>
#include <Poco/Net/StreamSocket.h>
#include <Poco/NumberParser.h>
#include <Poco/Net/DialogSocket.h>
#include <Poco/UUIDGenerator.h>

#include "workers/CZFSSenderWorker.h"
#include "workers/CZFSReceiverWorker.h"
#include "workers/CZFSendNotification.h"
#include "workers/CCleanerWorker.h"

#include "../CHelper.h"

#include <Poco/RegularExpression.h>

namespace service
{
    int CBackupSubsystem::getQueueSize()
    {
        return m_sendingQueue.size();
    }

    CBackupSubsystem::CBackupSubsystem() :
        m_zfs(Util::Application::instance().getSubsystem<service::CZFSService>())
    {
    }

    void CBackupSubsystem::stopTransfers()
    {
        Logger::root().notice("Clearing queue and stopping workers");
        m_aborted = true;
        clearQueue();
        for (auto & w : m_workers)
        {
            w->abort();
        }
        m_workers.clear();
    }

	const char* CBackupSubsystem::name() const
    {
        return "backup-subsystem";
    }

    void CBackupSubsystem::run()
    {
        Mutex::ScopedLock lock(m_runMutex);
        watch.start();
		const auto backupId = UUIDGenerator::defaultGenerator().create().toString();
        // Nowy start, informacja o przerwaniu jest zerowana
        m_aborted = false;

        // Przed każdą obsługą backupów wykonaj snapshot, jeśli jest wymagany
        if (m_takeBackupSnapshot)
        {
            makeBackup();
            Logger::root().notice("Snapshot created");
        }
        else
        {
            Logger::root().notice("I'm not taking nor deleting snapshots");
        }

        Logger::root().notice("Backup in master mode");

        // Wyślij backupy poprzez CZFSSenderWorker.
        // Funkcja zablokuje się na tym wywołaniu do czasu zakończenia wszystkich transferów.
        sendBackup(backupId);

        Logger::root().notice("Backup duration %i [s].", watch.elapsedSeconds());

        // Jeśli przerwano backupu ręcznie poprzez stopTransfers, zatrzymaj się
		// Jeśli jednorazowy start, to kończymy działanie funkcji.
        if (m_aborted || Util::Application::instance().config().getBool("backup.once", false))
        {
			return;
        }

		// Usuń stare snapshoty
		if (!CCleanerWorker::instance()->start())
        {
            Logger::root().notice("Waiting for CCleanerWorker to finish and running again");
            CCleanerWorker::instance()->join();
            CCleanerWorker::instance()->start();
        }

        // Jeśli w konfiguracji określono interwał lub godzinę startu, to uruchom timer.
        if (m_scheduling)
        {
            m_timer.restart(getTimerInterval());
        }
    }

    void CBackupSubsystem::sendBackup(const std::string& uuid)
    {
        if (m_slaveAddress.toString() == "0.0.0.0:0")
        {
            Logger::root().notice("Slave server isn't defined, not sending ");
            return;
        }

        auto rootName = m_zfs.getRootDataset()->getName();
        auto currentDatasets = m_zfs.listDatasets(*m_zfs.getRootDataset(), false, true, m_depth);
        currentDatasets.erase(std::remove_if(currentDatasets.begin(), currentDatasets.end(), [rootName](auto iter) { return iter->getName() == rootName; } ), currentDatasets.end());
        auto iter = currentDatasets.begin();
        while (iter != currentDatasets.end())
        {
            auto & ds = *iter;
            // Sprawdź czy ignorujemy podczas wysyłki do slave.
            if (!isDatasetAllowed(ds->getRelativeName()))
            {
				Logger::root().debug("Dataset '%s' jest ignorowany", ds->getName());
                iter = currentDatasets.erase(iter);
                continue;
            }

			CZFSService::attachSnapshots(ds, m_backupPrefix);
            Logger::root().information("inserting to queue: '%s'", ds->getRelativeName());
            ++iter;
        }

        // Generate set of tasks
        for (entity::CDatasetInfo::Ptr ds : currentDatasets)
        {
			Logger::root().debug("Add to queue '%s'", ds->getName());
			auto *pNotification = new CZFSendNotification(ds, m_slaveAddress);
			pNotification->setRetryCount(m_retryCount);
			m_sendingQueue.enqueueNotification(pNotification);
		}

        // Start threads
        setupSenderQueue();

        // Wait until threads process all commands in queue
        Logger::root().debug("Waiting for all task to complete");
        m_threadPool->joinAll();
        m_threadPool->collect();
        m_workers.clear();

		clearCommand(m_slaveAddress);
    }

    void CBackupSubsystem::initialize(Util::Application& app)
	{
        m_sendingQueue.clear();
        m_bufferSize = app.config().getUInt("backup.buffer_size", 1) * 1024;

        if (!app.config().getBool("backup.enabled", true))
		{
            Logger::root().warning("Backup syubsystem is disabled");
            return;
        }

        m_mode = app.config().getString("backup.mode") == "slave" ? Mode::slave : Mode::master;
		m_filterRegex = app.config().getString("backup.filter_regex", "");
        setSocketTimeout(app.config().getInt("backup.socket_timeout", 20));
        // Jeśli w ogóle włączona komunikacja między serwerami

        if (m_mode == Mode::slave)
        {
            auto interface = app.config().getString("backup.listen", "0.0.0.0:4000");

            if (interface.empty())
            {
                throw Poco::Exception("Slave mode must have have defined backup.listen parameter.");
            }

            m_threadPool.reset(new ThreadPool(1, std::max<int>(app.config().getInt("backup.threads", 1), 0)));
            setupFilter(app.config().getString("backup.allowed_ips", ""));
            setupTCPServer(Net::SocketAddress(interface));
        }
        else if (m_mode == Mode::master)
        {
			m_retryCount = app.config().getInt("backup.retry_count", 1);
            Logger::root().notice("Retry count: %?i", m_retryCount);

            auto addr = Util::Application::instance().config().getString("backup.slave_address", "");
            if (!addr.empty())
            {
                m_slaveAddress = Net::SocketAddress(addr);
            }

            m_backupPrefix = app.config().getString("backup.prefix");
            m_depth = app.config().getInt("backup.depth", 1);

			if (app.config().getBool("backup.recursive", true))
			{
                Logger::root().notice("Recursive mode, enforcing config option backup.depth = 1");
				m_depth = 1;
			}

            Logger::root().notice("Snapshot's prefix: '%s'", m_backupPrefix);
			setSnapshotsEnabled(app.config().getBool("backup.snapshot", false));

            if (!app.config().getBool("backup.once", false))
            {
                setupTimer(app.config());
            }
        }
	}

	void CBackupSubsystem::uninitialize()
	{
        m_timer.stop();

        if (!m_server.isNull())
        {
            m_server->stop();
            m_server.reset();
        }

        if (!m_threadPool.isNull())
        {
            Logger::root().notice("waiting for threads to complete");
            m_threadPool->joinAll();
        }
    }

    long CBackupSubsystem::getTimerInterval() const
    {
        if (!m_scheduling)
        {
            Logger::root().information("Timer is disabled");
            return 0;
        }

		LocalDateTime planned;
		LocalDateTime now;

        if (m_fixedInterval > 0)
        {
            planned = now + Timespan(0, 0, static_cast<int>(m_fixedInterval), 0, 0);
        }
        else
        {
            planned = LocalDateTime(now.year(), now.month(), now.day(), m_startHour, m_startMinute);

            if (planned <= now)
            {
                Logger::root().debug("After today's schedule - planning backup in next day");
                Timespan day(1, 0, 0, 1, 0);
                planned += day;
            }
        }

        long diff = (planned - now).totalMilliseconds();
        Logger::root().notice("Next backup at: %s", DateTimeFormatter::format(planned, "%Y-%m-%d %H:%M"));
        return diff;
    }

    /*!
     * \brief Wykonuje zwykły snapshot dla całego głównego datasetu wraz z podzbiorami. To jest mechanizm backupów.
     */
    void CBackupSubsystem::makeBackup()
    {
        Logger::root().notice("Beggining backup");
        const std::string backupName = m_backupPrefix + DateTimeFormatter::format(LocalDateTime(), "%Y-%m-%d-%H-%M-%S");
        Logger::root().notice("Creating recursive backup: '%s' in dataset '%s'", backupName, m_zfs.getRootDataset()->getName());
        service::CZFSService::createSnapshot(*m_zfs.getRootDataset(), backupName, true);
    }

    bool CBackupSubsystem::removeOldBackups()
    {
        if (!m_runMutex.tryLock())
        {
            Logger::root().debug("BackupSubsystem busy");
            return false;
        }

        Logger::root().debug("Running cleaner process");
        bool ret = CCleanerWorker::instance()->start();
        m_runMutex.unlock();
        return ret;
    }

	Net::TCPServerConnection* CBackupSubsystem::createConnection(const Net::StreamSocket& socket)
	{
        Net::DialogSocket dlg(socket);

        std::string action;
        dlg.receiveMessage(action);

        Logger::root().debug("Action: '%s'", action);

        // Jeśli backup, to odpal wątek klienta CZFSReceiver, który obsłuży połączenie.
        if (action == "backup")
        {
            auto thread = new CZFSReceiverWorker(socket);
			thread->setBufferSize(static_cast<int>(m_bufferSize));
            thread->setSocketTimeout(m_socketTimeout);
            thread->setAlwaysRollback(Util::Application::instance().config().getBool("backup/always_rollback", false));
            return thread;
        }
		else if (action == "clear")
		{
			if (removeOldBackups())
			{
				Logger::root().notice("Accepting clear command");
				dlg.sendMessage("OK");
			}
			else
			{
				Logger::root().warning("Rejecting clear command - busy");
				dlg.sendMessage("BUSY");
			}
		}

        return nullptr;
	}

	bool CBackupSubsystem::accept(const Net::StreamSocket& socket)
	{
        Logger::root().debug("Checking if client can connect: '%s'", socket.peerAddress().toString());
        return m_allowedIPS.empty() || std::find(m_allowedIPS.begin(), m_allowedIPS.end(), socket.address().toString()) != m_allowedIPS.end();
	}

    void CBackupSubsystem::clearQueue()
    {
        Logger::root().information("SendNotification queue cleared");
        m_sendingQueue.clear();
        m_sendingQueue.wakeUpAll();
    }

    void CBackupSubsystem::setupTCPServer(const Net::SocketAddress& addr) {
        Logger::root().notice("Backup in slave mode - awaiting connections on %s", addr.toString());
        m_server.reset(new Net::TCPServer(this, *m_threadPool, Net::ServerSocket(addr)));
        m_server->setConnectionFilter(this);
        m_server->start();
    }

    void CBackupSubsystem::setupSenderQueue()
    {
        Logger::root().notice("Number of tasks in queue: " + std::to_string(m_sendingQueue.size()));
        m_threadPool.reset(new ThreadPool(1, std::max<int>(Util::Application::instance().config().getInt("backup.threads", 1), 0)));

        while (m_threadPool->available() > 0)
        {
            auto *pWorker = new CZFSSenderWorker(&m_sendingQueue);
			pWorker->setBufferSize(m_bufferSize);
            pWorker->setSocketTimeout(m_socketTimeout);
			pWorker->setRecursive(Util::Application::instance().config().getBool("backup.recursive", true));
            m_workers.push_back(pWorker);
            m_threadPool->start(*pWorker);
        }
    }

    void CBackupSubsystem::setupFilter(const std::string& allowedIPS)
    {
        Logger::root().debug("Allowed IPs: '%s'", allowedIPS);
        StringTokenizer st(allowedIPS, ",", StringTokenizer::TOK_IGNORE_EMPTY | StringTokenizer::TOK_TRIM);

        for (int i = 0; i < st.count(); ++i)
        {
            m_allowedIPS.push_back(st[i]);
        }
    }

    void CBackupSubsystem::setupTimer(const Util::LayeredConfiguration &conf)
    {
        m_fixedInterval = conf.getInt("backup.interval", 0);

        // Jesli okreslony interwal, to robimy backupy co okreslony czas, a nie o okreslonej porze
        if (m_fixedInterval > 0)
        {
            Logger::root().notice("Running backups every  %i minutes", m_fixedInterval);
        }
        else
        {
            auto startTime = conf.getString("backup.time", "");

            if (!startTime.empty())
            {
                CHelper::getHourMinute(startTime, m_startHour, m_startMinute);
                Logger::root().notice("Backup start at: '%s'", startTime);
            }
            else
            {
                Logger::root().warning("Property backup.time not set - backups won't run from scheduler and will be available only on demand via AREST PI");
                m_scheduling = false;
                return;
            }
        }

        m_timer.setStartInterval(getTimerInterval());
        TimerCallback<CBackupSubsystem> backupCallback(*this, &CBackupSubsystem::timerCallback);
        m_timer.start(backupCallback);
    }

    void CBackupSubsystem::timerCallback(Timer &timer)
    {
        run();
    }

	std::set<std::string> CBackupSubsystem::setupIgnoreList(const std::string & ignoreStr)
	{
		std::set<std::string> ret;
		StringTokenizer st(ignoreStr, ",", StringTokenizer::TOK_TRIM | StringTokenizer::TOK_IGNORE_EMPTY);

		for (auto i = 0; i < st.count(); ++i)
		{
			ret.insert(st[i]);
		}

		return ret;
	}

    bool CBackupSubsystem::isDatasetAllowed(const std::string &name) const
    {
        if (m_filterRegex.empty())
        {
            return true;
        }
        // Jeśli nazwa datasetu nie pasuje do regex-a, to jest ignorowany
        RegularExpression regex(m_filterRegex);
        return regex.match(name, 0);
	}

    bool CBackupSubsystem::tryRun()
    {
        if (m_runMutex.tryLock())
        {
            TimerCallback<CBackupSubsystem> backupCallback(*this, &CBackupSubsystem::timerCallback);
            m_timer.stop();
			ThreadPool::defaultPool().start(*this);
			m_runMutex.unlock();
			return true;
        }

        return false;
    }

	void CBackupSubsystem::setSnapshotsEnabled(bool snapshotsEnabled)
	{
		if (m_backupPrefix.empty())
		{
			Logger::root().notice("Prefix for backup snapshots is not defined - snapshots are disabled!");
			m_takeBackupSnapshot = false;
			return;
		}

		m_takeBackupSnapshot = snapshotsEnabled;
		Logger::root().notice("Executing snapshot during backup is %s", std::string(m_takeBackupSnapshot ? "ENABLED" : "DISABLED - trying to use existing snapshots"));
	}

    void CBackupSubsystem::setSocketTimeout(int secs)
    {
        m_socketTimeout = std::max<int>(5, secs);
    }

    CBackupSubsystem::Mode CBackupSubsystem::getMode() const
    {
        return m_mode;
    }

    std::string CBackupSubsystem::getBackupPrefix() const
    {
        return m_backupPrefix;
    }

	void CBackupSubsystem::clearCommand(const Net::SocketAddress & slaveAddress) const
	{
		try
		{
			Net::DialogSocket sock;
			sock.connect(slaveAddress, Timespan(m_socketTimeout, 0));
			sock.setSendTimeout(Timespan(m_socketTimeout, 0));
			sock.setReceiveTimeout(Timespan(m_socketTimeout, 0));
			sock.sendMessage("clear");

			std::string response;
			if (sock.receiveMessage(response))
			{
				if (response == "OK")
				{
					Logger::root().notice("Clear command successfully started");
				}
				else
				{
					Logger::root().warning("Clear command error: '%s'", response);
				}
			}
		}
		catch (const std::exception & ex)
		{
			Logger::root().error("Clear command: '%s'", std::string(ex.what()));
		}
	}

}

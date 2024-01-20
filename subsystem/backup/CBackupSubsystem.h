//
// Created by zielin on 13.11.23.
//

#ifndef STORAGE_CBACKUPSUBSYSTEM_H
#define STORAGE_CBACKUPSUBSYSTEM_H

#include "../CZFSService.h"
#include <Poco/Stopwatch.h>
#include <Poco/ThreadPool.h>
#include <Poco/Util/Subsystem.h>
#include <Poco/Net/TCPServer.h>
#include <Poco/Net/TCPServerConnectionFactory.h>
#include <Poco/NotificationQueue.h>
#include <Poco/Timer.h>
#include <Poco/Mutex.h>
#include <Poco/Util/LayeredConfiguration.h>

#include "../../entity/CDatasetInfo.h"
#include <set>

using namespace Poco;

class CZFSSenderWorker;

namespace service
{
	class CBackupSubsystem
            : public Util::Subsystem, public Net::TCPServerConnectionFactory, public Net::TCPServerConnectionFilter, public Runnable
	{
        // ISenderCounter interface
    public:
        enum class Mode : int
        {
            master,
            slave
        };
        CBackupSubsystem();

        Mode getMode() const;

        void stopTransfers();
        int getQueueSize();

		const char* name() const override;
        /*!
         * \brief Uruchamia backup w sposób synchroniczny - bez timera, od razu. Blokuje m_runMutex w celu uniemożliwienia
         * uruchomienia wielu backupów (np. poprzez CLI).
         */
        void run() override;

		Net::TCPServerConnection* createConnection(const Net::StreamSocket& socket) override;
		bool accept(const Net::StreamSocket& socket) override;

        /*!
         * \brief Czysci kolejke zadan z backupami.
         */
        void clearQueue();

        /*!
         * \brief Weryfikuje nazwę datasetu na podstawie wyrażenia regularnego i zwraca true, jeśli
         * ma podlegać procedurze backupowania.
         * \param name nazwa datasetu.
         * \return true, jeśli chcemy, aby dataset był akceptowany podczas wysyłania.
         */
        bool isDatasetAllowed(const std::string & name) const;

        /*!
         * \brief Próbuje się zawiesić na muteksie m_runMutex i restartuje timer z opóźnieniem 1 sek.
         * @return true jeśli udało się uruchomić backup.
         */
        bool tryRun();

		/*!
		 * \brief Włącza lub wyłącza snapshoty backupów. Gdy prefix dla backupu nie jest ustawiony, to snapshoty
		 * zawsze będą wyłączone.
		 * @param snapshot
		 */
		void setSnapshotsEnabled(bool snapshot);

		/*!
		 * \brief Usuwa wszystkie backupy starsze niż backup.keep_days.
		 */
		bool removeOldBackups();

        std::string getBackupPrefix() const;

    protected:
		void initialize(Util::Application& app) override;
		void uninitialize() override;
        long getTimerInterval() const;

        /*!
         * \brief Wykonuje backupy. Do poprawnego działania potrzebne jest wykonanie backupu rekursywnego na
         * głównym datasecie, ponieoważ z niego pobierane są nazwy historycznych backupów (przy inicjalizacji, w trakcie
         * pracy).
         *
         */
        void makeBackup();

        void setSocketTimeout(int secs);
        void setupTCPServer(const Net::SocketAddress& addr);

        /*!
         * \brief Musimy wysylac tylko jednym watkiem w przypadku mastera.
         */
        void setupSenderQueue();
        void setupFilter(const std::string& allowedIPS);
		static std::set<std::string> setupIgnoreList(const std::string & ignoreStr);

        void setupTimer(const Util::LayeredConfiguration& conf);
        void timerCallback(Timer & timer);
		void sendBackup(const std::string& uuid);

        service::CZFSService& m_zfs;

        //! Ile dni wstecz trzymamy usunięte datasety na slave
        int m_missingDatasetDays = 7;
        int m_socketTimeout = 10;
		int m_retryCount = 1;
        unsigned int m_bufferSize = 1;

        ///! Podczas wywołania \a stopTransfers() ustawiane na true.
        ///! Podczas startu \a run() ustawiane na false.
        bool m_aborted = false;

        //! Jeśli true, w trakcie backupu jest robiony snapshot.
        //! Jeśli false, snapshot ZFS nie jest wykonywany.
        bool m_takeBackupSnapshot = true;

        //! Jeśli większe od zero, tzn., że backupy są robione co określony interwał, a nie o określonej porze.
        unsigned long long m_fixedInterval = 0;
        int m_startHour = 0;
        int m_startMinute = 0;
        //! Ustawiane na false jeśli nie określono interwału ani konkretnej godziny startu backupów.
        bool m_scheduling = true;

        Mode m_mode;

		SharedPtr<Net::TCPServer> m_server;
		SharedPtr<ThreadPool> m_threadPool;

		Net::SocketAddress m_slaveAddress;
        NotificationQueue m_sendingQueue;

        std::vector<std::string> m_allowedIPS;
        Timer m_timer;
        std::string m_backupPrefix;

        ///! Mutex do blokowania wielu wywołań mechanizmu backupów
        Mutex m_runMutex;
        int m_depth = 1;

        ///! Zawiera filtr, który pozwala przepuszczać dozwolone nazwy datasetów
		std::string m_filterRegex;
        Stopwatch watch;
        std::vector<CZFSSenderWorker*> m_workers;

		void clearCommand(const Net::SocketAddress & slaveAddress) const;
	};
}

#endif //STORAGE_CBACKUPSUBSYSTEM_H

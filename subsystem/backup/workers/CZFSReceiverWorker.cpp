//
// Created by zielin on 09.11.23.
//

#include "CZFSReceiverWorker.h"
#include "subsystem/backup/CBackupSubsystem.h"
#include "subsystem/backup/CTransferMonitor.h"
#include "subsystem/backup/CTransferNotificationSend.h"
#include "subsystem/backup/CTransferNotificationStart.h"
#include "subsystem/backup/CTransferNotificationStop.h"
#include "subsystem/backup/CTransferNotificationText.h"
#include "subsystem/CZFSService.h"
#include "CByteCounter.h"

#include <Poco/Util/Application.h>
#include <Poco/Process.h>
#include <Poco/Net/ServerSocket.h>
#include <Poco/PipeStream.h>
#include <Poco/String.h>
#include <Poco/Timespan.h>
#include <Poco/Logger.h>
#include <Poco/StringTokenizer.h>
#include <Poco/Net/DialogSocket.h>
#include <Poco/Stopwatch.h>
#include <Poco/UUIDGenerator.h>
#include <Poco/StreamCopier.h>
#include <sys/wait.h>

using namespace Poco;
using namespace Poco::Net;

CZFSReceiverWorker::CZFSReceiverWorker(const Net::StreamSocket & s) :
    Net::TCPServerConnection(s)
{
}

void CZFSReceiverWorker::run()
{
    auto& zfs = Util::Application::instance().getSubsystem<service::CZFSService>();
    entity::CDatasetInfo::Ptr targetDataset;
    const std::string uuid = UUIDGenerator::defaultGenerator().create().toString();
    auto rootDataset = zfs.getRootDataset();
    // Used during exception handling: when full transfer, target dataset is destroyed.
	bool fullTransfer = true;
	pid_t pid = 0;

	SharedPtr<PipeInputStream> err, in;
	SharedPtr<PipeOutputStream> os;

    try
	{
        CByteCounter counter;
        Stopwatch watch;
        watch.start();

        Logger::root().information("Connection from: '%s', timeout: %?d", socket().peerAddress().toString(), m_socketTimeout);
		socket().setSendTimeout(Timespan(m_socketTimeout, 0));
		socket().setReceiveTimeout(Timespan(m_socketTimeout, 0));

        auto session = negotiateRequest(socket());

        // skip = true means we have this backup(snapshot)
        if (session->skip)
        {
            socket().shutdown();
            socket().close();
            return;
        }

        // Target dataset
        Logger::root().debug("Making target dataset: " + session->name);
        targetDataset = rootDataset->createChildren(session->name);
		Process p;
		Process::Args args;
        args.emplace_back("zfs");
        args.emplace_back("receive");
        args.emplace_back("-u");
        args.emplace_back("-M");
        args.emplace_back("-o");
        args.emplace_back("overlay=on");
        args.emplace_back("-o");
        args.emplace_back("snapdir=visible");
		args.emplace_back("-o");
		args.emplace_back("sharenfs=none");
		args.emplace_back("-o");
		args.emplace_back("readonly=on");

        if (!rootDataset->getMountPoint().empty())
        {
            args.emplace_back("-o");
            args.emplace_back("mountpoint=" + targetDataset->getMountPoint());
        }

        // Always rollback on zfs-recv level.
		args.emplace_back("-F");

        // Full import, if target dataset doesn't exist on receiver or there is no
        // snapshot which can be used as start point in incremental transfer
		if (session->startSnapshot.empty())
		{
            Logger::root().notice("Receiving full dataset '%s'", targetDataset->getName());
		}
        else
        {
			fullTransfer = false;
            Logger::root().notice("Incremental '%s' from '%s'", targetDataset->getName(), session->startSnapshot);
            if (m_alwaysRollback)
            {
                Logger::root().information("(Always) rollback until '%s@%s'", targetDataset->getName(), session->startSnapshot);
                service::CZFSService::rollback(targetDataset, session->startSnapshot);
            }
            else
            {
                Logger::root().debug("Always rollback is disabled");
            }
        }

		args.push_back(targetDataset->getName());
		Pipe pipe;
        Pipe pipe2;
		Pipe errPipe;

		err.reset(new PipeInputStream(errPipe));
		os.reset(new PipeOutputStream(pipe));
		in.reset(new PipeInputStream(pipe2));

		Logger::root().information("sudo " + cat<std::string>(" ", args.cbegin(), args.cend()));
		auto handle = Poco::Process::launch("sudo", args, &pipe, &pipe2, &errPipe);


        if (handle.id() == 0)
        {
            throw Poco::Exception("Can't launch process");
        }

        if (handle.tryWait() != -1)
        {
            throw Poco::Exception("ZFS receiver not started");
        }

		pid = handle.id();
        SharedPtr<char> buff(new char [m_bufferSize]);
        int seconds = 0;

        CTransferMonitor::instance()->send(new CTransferNotificationStart(targetDataset, session, uuid, false));

        int bytes;

        while ( Poco::Process::isRunning(handle) && (bytes = socket().receiveBytes(buff, m_bufferSize)) > 0 )
        {
            counter.addBytes(bytes);
            os->write(buff, bytes);
            if (seconds + 2 < watch.elapsedSeconds())
            {
                CTransferMonitor::instance()->send(new CTransferNotificationSend(targetDataset, uuid, counter));
                seconds = watch.elapsedSeconds();
            }
        }

        os->close();
        in->close();
		err->close();

        Process::wait(handle);

        Logger::root().notice("Transfer of '%s' is finished. Processsed %s in %s [s].", targetDataset->getName(), counter.toString(), std::to_string(watch.elapsedSeconds()));
        CTransferMonitor::instance()->send(new CTransferNotificationStop(targetDataset, uuid, counter, watch.elapsedSeconds()));
		return;
	}
	catch (const Poco::Exception& ex)
    {
        Logger::root().error("CZFSReceiver: (" + (targetDataset.isNull() ? "unknown dataset" : targetDataset->getName()) + ") - " + ex.displayText());
        CTransferMonitor::instance()->send(new CTransferNotificationText(uuid, "CZFSReceiver: (" + (targetDataset.isNull() ? "unknown dataset" : targetDataset->getName()) + ") - " + ex.displayText()));

		std::stringstream ss;
		StreamCopier::copyStream(*err, ss);

		Logger::root().error("CZFSReceiver: (" + (targetDataset.isNull() ? "unknown dataset" : targetDataset->getName()) + ") - '%s'", Poco::trim(ss.str()));

		if (pid > 1)
		{
			int wstatus;
			waitpid(pid,&wstatus, WUNTRACED | WCONTINUED);
		}
	}

    // In case of error, destroy dataset
	if (fullTransfer && !targetDataset.isNull())
	{
        Logger::root().information("Full transfer failed - destroying dataset");
		zfs.destroyDataset(targetDataset);
    }
}

void CZFSReceiverWorker::setAlwaysRollback(bool newAlwaysRollback)
{
    m_alwaysRollback = newAlwaysRollback;
}

CTransferSession::Ptr CZFSReceiverWorker::negotiateRequest(StreamSocket &socket)
{
    Logger::root().information("Negotiation stared");
	auto & zfs = Util::Application::instance().getSubsystem<service::CZFSService>();
    CTransferSession::Ptr ret(new CTransferSession);
    Logger::root().debug("Waiting for transmission details");
    DialogSocket dlg(socket);
    dlg.sendMessage("OK");
    std::string msg;
    std::vector<std::string> snapshots;

    while (dlg.receiveMessage(msg))
    {
        if (msg.empty())
        {
            break;
        }
		Logger::root().debug("Got: '%s'", msg);
        StringTokenizer st(msg, "=", StringTokenizer::TOK_TRIM);

        if (st.count() != 2)
        {
            dlg.sendMessage("-ERR unknown command: " + msg);
            throw Poco::Exception("Client sent unknown command: " + msg);
        }
        else
        {
            if (st[0] == "name" && !st[1].empty())
            {
				Logger::root().debug("Dataset name '%s'", st[1]);
                ret->name = st[1];
            }
            else if (st[0] == "snapshot")
            {
                snapshots.emplace_back(st[1]);
                Logger::root().debug("Master snapshot: " + st[1]);
            }
        }

        dlg.sendMessage("OK");
    }

    // stopSnapshot snapshot niezależnie czy przyrostowo czy pełny, zawsze jest ostatnim elementem na liście odebranych z socketu.
    ret->stopSnapshot = snapshots.back();

    if (ret->name.empty())
	{
		dlg.sendMessage("error=Dataset is not defined");
        throw Poco::Exception("Client '" + socket.peerAddress().toString() + "' - dataset name is required");
	}

	Logger::root().debug("Checking '%s' on our side", ret->name);
    auto children = zfs.getRootDataset()->createChildren(ret->name);

    // Docelowy dataset, jeśli istnieje. Pobierz jego snapshoty w celu ustalenia czy można zrobić przyrostowo.
    ret->targetDataset = zfs.listDataset(children->getName(), true);

    // Jeśli docelowy dataset istnieje to:
    // - jeśli przesyłany backup zawiera snapshoty, sprawdź czy można zrobić przyrostowo.
    // - spośród odebranych znajdź taki, który pasuje do naszego ostatniego i nie jest ostatnim w snapshotach nadawcy.
    if (!ret->targetDataset.isNull() && !snapshots.empty() && !ret->targetDataset->getSnapshots().empty())
    {
        // Snapshot, od którego trzeba zacząć lub pusty w przypadku pełnego transferu.
        ret->startSnapshot = getStartPoint(*ret->targetDataset, snapshots);

        Logger::root().debug("Start snapshot: '%s'", ret->startSnapshot);

        // Jeśli coś znaleziono, to sprawdź czy ostatni snapshot na slave jest ostatnim snap. na masterze...
        if (!ret->startSnapshot.empty())
        {
            // ... jeśli jest to ustaw skip=true i zasygnalizuj, że nie chcemy transferu
            if (ret->startSnapshot == snapshots.back())
            {
                ret->skip = true;
                dlg.sendMessage("skip=true");
                Logger::root().information("Omit - '%s@%s'", ret->targetDataset->getName(), ret->startSnapshot);
            }
            else
            {
                ret->skip = false;
                Logger::root().information("Backup of '%s' begins from '%s'", ret->targetDataset->getName(), ret->startSnapshot);
            }
        }
    }

    // Jeśli nie ustawiono, skip i określono snapshot, tzn., że robimy przyrosotwy backup.
    if (!ret->skip)
    {
        if (!ret->startSnapshot.empty())
        {
			Logger::root().debug("We want incremental from '%s'", ret->startSnapshot);
            // Notify sender that we want this snapshot
            dlg.sendMessage("snapshot=" + ret->startSnapshot);
        }
    }

    auto & backup = Util::Application::instance().getSubsystem<service::CBackupSubsystem>();

    if (!backup.isDatasetAllowed(children->getRelativeName()))
    {
        Logger::root().notice("Dataset '%s' is ignored", children->getRelativeName());
        ret->skip = true;
    }

    dlg.sendMessage("");
	Logger::root().information("Negotiation finished");
	return ret;
}

std::string CZFSReceiverWorker::getStartPoint(const entity::CDatasetInfo &targetDataset, const std::vector<std::string> & snapshots)
{
	std::string found;
	const auto & snaps = targetDataset.getSnapshots();

	// Przjedź po oferowanych snapshotach
	for (const auto & offered : snapshots)
	{
		// Przejdź po snapshotach lokalnych
		for (const auto & local : snaps)
		{
			if (local->getName() == offered)
			{
				found = offered;
				break;
			}
		}
	}

	Logger::root().debug("Proposed snapshot: '%s'", found);
    return found;
}

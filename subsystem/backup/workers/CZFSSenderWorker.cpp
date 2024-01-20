//
// Created by zielin on 14.11.23.
//

#include "CZFSSenderWorker.h"
#include "CZFSendNotification.h"
#include "subsystem/backup/CTransferMonitor.h"
#include "subsystem/backup/CTransferNotificationText.h"
#include "subsystem/backup/CTransferNotificationSend.h"
#include "subsystem/backup/CTransferNotificationStart.h"
#include "subsystem/backup/CTransferNotificationStop.h"

#include <Poco/Logger.h>
#include <Poco/Net/StreamSocket.h>
#include <Poco/Net/DialogSocket.h>
#include <Poco/StringTokenizer.h>
#include <Poco/Process.h>
#include <Poco/Pipe.h>
#include <Poco/String.h>
#include <Poco/Stopwatch.h>
#include <Poco/UUIDGenerator.h>
#include <Poco/Thread.h>

void CZFSSenderWorker::run()
{
    Logger::root().debug("Worker thread started");

    while (!m_abort)
    {
        auto notification = m_queue->waitDequeueNotification(500);

		if (notification == nullptr)
        {
			break;
		}

		auto sendTask = dynamic_cast<CZFSendNotification*>(notification);

		if (sendTask != nullptr)
		{
			if (sendTask->isRetryFlagSet())
			{
				Logger::root().notice("Running again teransfer of '%s'", sendTask->getDataset()->getName());
			}

			if (!sendDataset(sendTask->getDataset(), sendTask->getAddr(), sendTask->isRetryFlagSet()))
			{
				if (sendTask->retry())
				{
					Logger::root().warning("Transfer '%s' will be started again later",
							sendTask->getDataset()->getName());
					m_queue->enqueueNotification(notification);
				}
			}
		}
    }

    Logger::root().information("Exiting thread");
}

CZFSSenderWorker::CZFSSenderWorker(NotificationQueue *queue) :
    m_queue(queue)
{
    Logger::root().debug("Sending thread ready");
}

bool CZFSSenderWorker::sendDataset(const entity::CDatasetInfo::Ptr & dataset, const Net::SocketAddress && m_addr, bool retry) const
{
	Logger::root().information("Sending '%s' to '%s'", dataset->getName(), m_addr.toString());

	const std::string uuid = UUIDGenerator::defaultGenerator().create().toString();
	CByteCounter counter;

	Stopwatch watch;
	watch.start();

	Net::StreamSocket sock;

	try
    {
        const auto &snapshots = dataset->getSnapshots();

        if (snapshots.empty())
        {
            Logger::root().debug("Dataset '%s' doesn't contain any snapshot",  dataset->getName());
            return true;
        }

        sock.connect(m_addr, Timespan(m_socketTimeout, 0));
		sock.setSendTimeout(Timespan(m_socketTimeout, 0));
		sock.setReceiveTimeout(Timespan(m_socketTimeout, 0));


        auto session = negotiate(dataset, sock);

        CTransferMonitor::instance()->send(new CTransferNotificationStart(dataset, session, uuid, retry));

        if (session->skip)
        {
            sock.close();
            Logger::root().information("Backup already exists");
            CTransferMonitor::instance()->send(new CTransferNotificationText(dataset, uuid, "We have the newest backup"));
            CTransferMonitor::instance()->send(new CTransferNotificationStop(dataset, uuid, counter, watch.elapsedSeconds()));
            return true;
        }

        Process::Args args;
        args.emplace_back("zfs");
        args.emplace_back("send");

		if (m_recursive)
		{
			Logger::root().information("Backup with descendant (-R option)");
			args.emplace_back("-R");
		}

        if (!session->startSnapshot.empty())
        {
            Logger::root().notice("Transfer of '%s' from '%s'",dataset->getRelativeName(), session->startSnapshot);
            args.emplace_back("-I");  // Ew. sprawdzić.

            args.emplace_back(dataset->getName() + "@" + session->startSnapshot);
            args.emplace_back(dataset->getName() + "@" + snapshots.back()->getName());
        }
        else
        {
            Logger::root().notice("Full backup '%s'", dataset->getRelativeName());
            args.emplace_back(dataset->getName() + "@" + snapshots.back()->getName());
        }

        Logger::root().information("sudo " + cat<std::string>(" ", args.cbegin(), args.cend()));

		Pipe pipe;
		auto handle = Process::launch("sudo", args, nullptr, &pipe, nullptr);
		SharedPtr<char> buff(new char[m_bufferSize]);
		int seconds = 0;

		// Dopoki proces dziala i nie wywolano przerwania
		while ( Process::isRunning(handle) )
        {
			if (m_abort)
			{
				killZFS(handle);
				break;
			}

			int bytes = pipe.readBytes(buff, static_cast<int>(m_bufferSize));

			// Zero means soocket closed
			if (bytes <= 0)
            {
				break;
			}

			int sent = 0;
			int offset = 0;
			// Wyślij cały bufor
			while (offset < bytes)
            {
                sent = sock.sendBytes(buff + offset,  bytes - offset);

                if (sent == 0)
                {
                    break;
                }
                if (sent < 0)
                {
                    Process::requestTermination(handle.id());
                    throw Poco::Exception("Transmission error");
                }

                offset += sent;
                counter.addBytes(sent);

                if (seconds + 1 < watch.elapsedSeconds())
                {
                    CTransferMonitor::instance()->send(new CTransferNotificationSend(dataset, uuid, counter));
                    seconds = watch.elapsedSeconds();
                }
            }
        }

        CTransferMonitor::instance()->send(new CTransferNotificationStop(dataset, uuid, counter, watch.elapsedSeconds()));
		killZFS(handle);
		return true;
    }
    catch (const Poco::Exception & ex)
    {
        Logger::root().error("Dataset: '%s', error: '%s', object: '%s', timeout: %s [s].",
				dataset->getName(),
				ex.displayText() ,
				std::string(ex.className()),
				std::to_string(sock.getSendTimeout().totalSeconds()));
    }

	CTransferMonitor::instance()->send(new CTransferNotificationStop(dataset, uuid, counter, watch.elapsedSeconds()));
	return false;
}

CTransferSession::Ptr CZFSSenderWorker::negotiate(const entity::CDatasetInfo::Ptr &dataset, Net::StreamSocket &sock)
{
	Logger::root().information("Negotiation started");

	std::string res;
    Net::DialogSocket dlg(sock);

    dlg.sendMessage("backup");
    dlg.receiveMessage(res);

	Logger::root().debug("Sending '%s' (relative: '%s')", dataset->getName(), dataset->getRelativeName());

    dlg.sendMessage("name=" + dataset->getRelativeName());
    dlg.receiveMessage(res);

    for (auto & s : dataset->getSnapshots())
    {
		Logger::root().debug("Proposing snapshot: '%s'", s->getName());
        dlg.sendMessage("snapshot=" + s->getName());
        dlg.receiveMessage(res);
    }

    // Pusty wiersz oznacza koniec żądania
    dlg.sendMessage("");
    std::string response;

    CTransferSession::Ptr session(new CTransferSession);
    // Last snapshot - always last element.
    session->stopSnapshot = dataset->getSnapshots().back()->getName();

    do
    {
        dlg.receiveMessage(response);

        // Pusta linia oznacza koniec komunikacji.
        if (response.empty())
        {
            break;
        }

		Logger::root().debug("Got: '%s'", response);
        StringTokenizer st(response, "=");

        if (st.count() != 2)
        {
            dlg.sendMessage("error=syntax");
            throw Poco::Exception("Incorrect response: " + response);
        }

        if (st[0] == "snapshot" && !st[1].empty())
        {
            session->startSnapshot = st[1];
        }
        else if (st[0] == "skip")
        {
            session->skip = true;
			Logger::root().information("Negotiation finished");
			return session;
        }
    }
    while (!response.empty());

	Logger::root().information("Negotiation finished");
    return session;
}

void CZFSSenderWorker::setRecursive(bool isRecursive)
{
	m_recursive = isRecursive;
}

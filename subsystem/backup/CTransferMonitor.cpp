#include "CTransferMonitor.h"

#include "CTransferNotificationSend.h"
#include "CTransferNotificationText.h"
#include "CTransferNotificationStop.h"
#include "CTransferNotificationStart.h"

#include <Poco/Logger.h>

CTransferMonitor *CTransferMonitor::instance()
{
    static SingletonHolder<CTransferMonitor> instance;
    return instance.get();
}

CTransferMonitor::~CTransferMonitor()
{
	stop();
}

void CTransferMonitor::run()
{
    Logger::root().debug("CTransferObserver started");

    while (true)
    {
        auto n = dynamic_cast<CTransferNotification*>(m_queue.waitDequeueNotification());

        if (n == nullptr)
        {
            break;
        }

        switch (n->getAction())
        {
        case CTransferNotification::Action::start:
            {
                auto* start = dynamic_cast<CTransferNotificationStart*>(n);
                auto* info = new CTransferInfo;
				info->setRetry(start->isRetryFlagSet());
                info->setId(start->getUUID());
                info->setDataset(start->getDataset());
                info->setStartTime(start->getStartTime().timestamp());
                info->setLastUpdate(start->getStartTime().timestamp());
                info->setStartSnapshot(start->session()->startSnapshot);
                info->setStopSnapshot(start->session()->stopSnapshot);
                info->setState(CTransferInfo::State::running);
                FastMutex::ScopedLock lock(m_mtx);
                m_transfers[info->getId()] = info;
            }
        break;

        case CTransferNotification::Action::transfer:
            {
                auto* sent = dynamic_cast<CTransferNotificationSend*>(n);
                FastMutex::ScopedLock lock(m_mtx);
                auto info = m_transfers[sent->getUUID()];
                info->setCounter(sent->getCounter());
                info->setLastUpdate(Timestamp());
                info->setState(CTransferInfo::State::running);
//                Logger::root().debug("SENT: %s - %s", sent->getDataset()->getName(), sent->getCounter().toString());
            }
        break;

        case CTransferNotification::Action::info:
            {
                auto *text = dynamic_cast<CTransferNotificationText*>(n);
                FastMutex::ScopedLock lock(m_mtx);
                auto info = m_transfers[text->getUUID()];
                info->addMessage(text->getText());
            }
        break;

        case CTransferNotification::Action::stop:
            {
                auto *stop = dynamic_cast<CTransferNotificationStop*>(n);
                FastMutex::ScopedLock lock(m_mtx);
                auto info = m_transfers[stop->getUUID()];

				if (info.isNull())
				{
					Logger::root().warning("Can't find any transfer to set 'stop' status");
					return;
				}

                info->setStopTime(Timestamp());
                info->setState(CTransferInfo::State::finished);
            }
        }

        delete n;
    }

    Logger::root().information("CTransferObserver stopped");
}

void CTransferMonitor::send(CTransferNotification *notification)
{
    m_queue.enqueueNotification(notification);
}

void CTransferMonitor::getTransfers(std::ostream & os, CTransferInfo::State state)
{
    FastMutex::ScopedLock lock(m_mtx);

	for (const auto & i : m_transfers)
    {
		// Filtrowanie statusów
		if (state == CTransferInfo::State::any || i.second->getState() == state)
		{
			const auto & t = (*i.second);
			os 	<< t.getDatasetName() << "\t" << t.getCounter().toString() << std::endl
				<< DateTimeFormatter::format(t.getStartTime(), "yyyy-MM-dd HH:mm:ss") << std::endl
				<< DateTimeFormatter::format(t.getLastUpdate(), "yyyy-MM-dd HH:mm:ss") << std::endl
				<< DateTimeFormatter::format(t.getStopTime(), "yyyy-MM-dd HH:mm:ss") << std::endl;
		}
    }
}

CTransferMonitor::CTransferMonitor()
{
    m_thread.start(*this);
}

void CTransferMonitor::clear()
{
    FastMutex::ScopedLock lock(m_mtx);
    auto iter = m_transfers.begin();
    while (iter != m_transfers.end())
    {
        if (iter->second->getState() == CTransferInfo::State::finished)
        {
            iter = m_transfers.erase(iter);
            continue;
        }

        ++iter;
    }
}

void CTransferMonitor::stop()
{
	m_queue.wakeUpAll();
	m_thread.join();
}

JSON::Object::Ptr CTransferMonitor::toJSON()
{
	JSON::Object::Ptr ret = new JSON::Object;
	FastMutex::ScopedLock lock(m_mtx);
	JSON::Array::Ptr finished = new JSON::Array;
	JSON::Array::Ptr running = new JSON::Array;
	JSON::Array::Ptr waiting = new JSON::Array;

	ret->set("finished", finished);
	ret->set("waiting", waiting);
	ret->set("running", running);

	for (const auto & t : m_transfers)
	{
		switch (t.second->getState())
		{
		case CTransferInfo::State::waiting:
			waiting->add(t.second->toJSON());
			break;
		case CTransferInfo::State::running:
			running->add(t.second->toJSON());
			break;
		case CTransferInfo::State::finished:
			finished->add(t.second->toJSON());
			break;
		}
	}

	return ret;
}

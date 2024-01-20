//
// Created by zielin on 14.11.23.
//

#ifndef STORAGE_CSENDERWORKER_H
#define STORAGE_CSENDERWORKER_H

#include "entity/CDatasetInfo.h"
#include "subsystem/backup/workers/CTransferSession.h"

#include <Poco/SharedPtr.h>
#include <Poco/Runnable.h>
#include <Poco/NotificationQueue.h>
#include <Poco/Net/SocketAddress.h>
#include <Poco/Net/StreamSocket.h>
#include <Poco/Process.h>
#include <Poco/Net/DialogSocket.h>
#include "CWorker.h"

using namespace Poco;

class CZFSSenderWorker : public CWorker, public Runnable
{
public:
    explicit CZFSSenderWorker(NotificationQueue* queue);
    void run() override;
	void setRecursive(bool isRecursive);

protected:
    [[nodiscard]] bool sendDataset(const entity::CDatasetInfo::Ptr & dataset, const Net::SocketAddress && m_addr, bool retry) const;
    static CTransferSession::Ptr negotiate(const entity::CDatasetInfo::Ptr &dataset, Net::StreamSocket &sock);

    NotificationQueue* m_queue;
	bool m_recursive = true;
};


#endif //STORAGE_CSENDERWORKER_H

//
// Created by zielin on 09.11.23.
//

#ifndef STORAGE_CZFSRECEIVER_H
#define STORAGE_CZFSRECEIVER_H

#include "CWorker.h"
#include "entity/CDatasetInfo.h"
#include "CTransferSession.h"

#include <Poco/Runnable.h>
#include <Poco/Net/TCPServerConnection.h>
#include <Poco/Net/StreamSocket.h>
#include <Poco/PipeStream.h>

using namespace Poco;

class CZFSReceiverWorker : public CWorker, public Net::TCPServerConnection
{
public:
    explicit CZFSReceiverWorker(const Net::StreamSocket & s);
	void run() override;
    void setAlwaysRollback(bool newAlwaysRollback);

protected:
	Poco::Net::IPAddress m_addr;
    bool m_alwaysRollback = false;
    static CTransferSession::Ptr negotiateRequest(Net::StreamSocket &socket);
    static std::string getStartPoint(const entity::CDatasetInfo &targetDataset, const std::vector<std::string> &snapshots);
};


#endif //STORAGE_CZFSRECEIVER_H

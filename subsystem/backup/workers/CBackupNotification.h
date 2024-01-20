//
// Created by zielin on 06.01.24.
//

#ifndef STORAGE_CBACKUPNOTIFICATION_H
#define STORAGE_CBACKUPNOTIFICATION_H

#include <Poco/Notification.h>
#include <Poco/Net/SocketAddress.h>

using namespace Poco;

class CBackupNotification : public Notification
{
public:
	explicit CBackupNotification(const Net::SocketAddress& addr);

	Net::SocketAddress getAddr() const;

protected:
	Net::SocketAddress m_addr;
};


#endif //STORAGE_CBACKUPNOTIFICATION_H

//
// Created by zielin on 06.01.24.
//

#include "CBackupNotification.h"

CBackupNotification::CBackupNotification(const Net::SocketAddress& addr) :
	m_addr(addr)
{

}

Net::SocketAddress CBackupNotification::getAddr() const
{
	return m_addr;
}

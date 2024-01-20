//
// Created by zielin on 14.11.23.
//

#include "CZFSendNotification.h"

CZFSendNotification::CZFSendNotification(const entity::CDatasetInfo::Ptr& dataset, const Net::SocketAddress& addr) :
		CBackupNotification(addr),
		m_dataset(dataset)
{
}

const entity::CDatasetInfo::Ptr &CZFSendNotification::getDataset() const
{
    return m_dataset;
}

void CZFSendNotification::setDataset(const entity::CDatasetInfo::Ptr &dataset)
{
    m_dataset = dataset;
}

std::string CZFSendNotification::name() const
{
    return "zfs-send";
}

bool CZFSendNotification::retry()
{
	if (m_retryCount > 0)
	{
		m_retryFlag = true;
		--m_retryCount;
		return true;
	}

	return false;
}

bool CZFSendNotification::isRetryFlagSet() const
{
	return m_retryFlag;
}

void CZFSendNotification::setRetryCount(int cnt)
{
	m_retryCount = cnt;
}

int CZFSendNotification::getRetryCount() const
{
	return m_retryCount;
}

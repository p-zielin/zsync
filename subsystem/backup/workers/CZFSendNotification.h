//
// Created by zielin on 14.11.23.
//

#ifndef STORAGE_CZFSENDNOTIFICATION_H
#define STORAGE_CZFSENDNOTIFICATION_H

#include "entity/CDatasetInfo.h"
#include "CBackupNotification.h"
#include <Poco/Net/SocketAddress.h>

using namespace Poco;

class CZFSendNotification : public CBackupNotification
{
public:
    explicit CZFSendNotification(const entity::CDatasetInfo::Ptr & dataset, const Net::SocketAddress& addr);

    const entity::CDatasetInfo::Ptr &getDataset() const;
    void setDataset(const entity::CDatasetInfo::Ptr &dataset);

    std::string name() const override;
	bool retry();

	bool isRetryFlagSet() const;

	void setRetryCount(int cnt);
	int getRetryCount() const;

protected:
	entity::CDatasetInfo::Ptr m_dataset;
	int m_retryCount = 1;
	bool m_retryFlag = false;
};


#endif //STORAGE_CZFSENDNOTIFICATION_H

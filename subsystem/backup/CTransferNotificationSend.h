#ifndef CTRANSFERNOTIFICATIONSEND_H
#define CTRANSFERNOTIFICATIONSEND_H

#include "CTransferNotification.h"
#include "CByteCounter.h"

class CTransferNotificationSend : public CTransferNotification
{
public:
    CTransferNotificationSend(const entity::CDatasetInfo::Ptr & ds, const std::string &uuid, const CByteCounter &counter);

    Action getAction() const override;
    CByteCounter getCounter() const;

protected:
    CByteCounter m_counter;
};

#endif // CTRANSFERNOTIFICATIONSEND_H

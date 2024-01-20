#ifndef CTRANSFERNOTIFICATIONSTOP_H
#define CTRANSFERNOTIFICATIONSTOP_H

#include "CTransferNotification.h"

class CTransferNotificationStop : public CTransferNotification
{
public:
    CTransferNotificationStop(const entity::CDatasetInfo::Ptr &ds, const std::string & uuid, const CByteCounter &counter, unsigned long long seconds);
    CByteCounter getCounter() const;

    unsigned long long getSeconds() const;

protected:
    Action getAction() const override;
    CByteCounter m_counter;
    unsigned long long m_seconds = 0;
};

#endif // CTRANSFERNOTIFICATIONSTOP_H

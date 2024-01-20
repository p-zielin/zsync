#include "CTransferNotificationStop.h"

CTransferNotificationStop::CTransferNotificationStop(const entity::CDatasetInfo::Ptr &ds, const std::string &uuid, const CByteCounter &counter, unsigned long long seconds) :
    CTransferNotification(uuid, ds),
    m_counter(counter),
    m_seconds(seconds)
{

}

CByteCounter CTransferNotificationStop::getCounter() const
{
    return m_counter;
}

CTransferNotification::Action CTransferNotificationStop::getAction() const
{
    return Action::stop;
}

unsigned long long CTransferNotificationStop::getSeconds() const
{
    return m_seconds;
}

#include "CTransferNotificationSend.h"

CTransferNotificationSend::CTransferNotificationSend(const entity::CDatasetInfo::Ptr &ds, const std::string & uuid, const CByteCounter &counter) :
    CTransferNotification(uuid, ds),
    m_counter(counter)
{

}

CTransferNotification::Action CTransferNotificationSend::getAction() const
{
    return Action::transfer;
}

CByteCounter CTransferNotificationSend::getCounter() const
{
    return m_counter;
}

#include "CTransferNotificationStart.h"

CTransferNotificationStart::CTransferNotificationStart(const entity::CDatasetInfo::Ptr &ds, CTransferSession::Ptr &session, const std::string &uuid, bool retry) :
    CTransferNotification(uuid, ds),
    m_session(session),
	m_retry(retry)
{

}

CTransferNotification::Action CTransferNotificationStart::getAction() const
{
    return CTransferNotification::Action::start;
}

LocalDateTime CTransferNotificationStart::getStartTime() const
{
    return m_startTime;
}

CTransferSession::Ptr CTransferNotificationStart::session() const
{
    return m_session;
}

bool CTransferNotificationStart::isRetryFlagSet() const
{
	return m_retry;
}

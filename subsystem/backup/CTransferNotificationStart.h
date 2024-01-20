#ifndef CTRANSFERNOTIFICATIONSTART_H
#define CTRANSFERNOTIFICATIONSTART_H

#include "CTransferNotification.h"
#include "subsystem/backup/workers/CTransferSession.h"

class CTransferNotificationStart : public CTransferNotification
{
public:
    CTransferNotificationStart(const entity::CDatasetInfo::Ptr & ds, CTransferSession::Ptr & session, const std::string & uuid, bool retry);
    Action getAction() const override;
	LocalDateTime getStartTime() const;

    CTransferSession::Ptr session() const;
	bool isRetryFlagSet() const;
protected:
    LocalDateTime m_startTime;
    CTransferSession::Ptr m_session;
	bool m_retry;
};

#endif // CTRANSFERNOTIFICATIONSTART_H

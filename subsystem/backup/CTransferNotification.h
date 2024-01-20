#ifndef CTRANSFERNOTIFICATION_H
#define CTRANSFERNOTIFICATION_H

#include "CByteCounter.h"
#include "entity/CDatasetInfo.h"
#include <Poco/Notification.h>

class CTransferNotification : public Poco::Notification
{
public:
    typedef Poco::SharedPtr<CTransferNotification> Ptr;

    enum class Action
    {
        info,
        transfer,
        start,
        stop
    };

    CTransferNotification(const std::string & uuid, const entity::CDatasetInfo::Ptr &ds);
    virtual Action getAction() const = 0;

    std::string name() const override;
    entity::CDatasetInfo::Ptr getDataset() const;

    std::string getUUID() const;

protected:
    std::string m_uuid;
    entity::CDatasetInfo::Ptr m_ds;
    CByteCounter m_counter;
};

#endif // CTRANSFERNOTIFICATION_H

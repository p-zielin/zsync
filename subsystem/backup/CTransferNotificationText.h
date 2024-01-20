#ifndef CTRANSFERNOTIFICATIONTEXT_H
#define CTRANSFERNOTIFICATIONTEXT_H

#include "CTransferNotification.h"

class CTransferNotificationText : public CTransferNotification
{
public:
    CTransferNotificationText(const std::string & uuid, const std::string & text);
    CTransferNotificationText(const entity::CDatasetInfo::Ptr &ds, const std::string & uuid, std::string  text);

    Action getAction() const override;
    std::string getText() const;

protected:
    std::string m_text;
};

#endif // CTRANSFERNOTIFICATIONTEXT_H

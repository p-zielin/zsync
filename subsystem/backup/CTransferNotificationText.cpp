#include "CTransferNotificationText.h"

#include <utility>

CTransferNotificationText::CTransferNotificationText(const entity::CDatasetInfo::Ptr &ds, const std::string &uuid, std::string text) :
    CTransferNotification(uuid, ds),
    m_text(std::move(text))
{

}

CTransferNotification::Action CTransferNotificationText::getAction() const
{
    return Action::info;
}

std::string CTransferNotificationText::getText() const
{
    return m_text;
}

CTransferNotificationText::CTransferNotificationText(const std::string &uuid, const std::string &text) :
    CTransferNotification(uuid, nullptr),
	m_text(text)
{

}

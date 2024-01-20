#include "CTransferNotification.h"

CTransferNotification::CTransferNotification(const std::string &uuid, const entity::CDatasetInfo::Ptr &ds) :
    m_uuid(uuid),
    m_ds(ds)
{

}

std::string CTransferNotification::name() const
{
    return "Aktualizacja statusu transferu";
}

entity::CDatasetInfo::Ptr CTransferNotification::getDataset() const
{
    return m_ds;
}

std::string CTransferNotification::getUUID() const
{
    return m_uuid;
}

#include "CSnapshotInfo.h"

namespace entity
{

CSnapshotInfo::CSnapshotInfo()
{

}

CSnapshotInfo::CSnapshotInfo(const std::string &name) :
    m_name(name)
{

}

std::string CSnapshotInfo::getName() const
{
    return m_name;
}

void CSnapshotInfo::setName(const std::string &newName)
{
    m_name = newName;
}

Timestamp CSnapshotInfo::getCreationTime() const
{
    return m_creationTime;
}

void CSnapshotInfo::setCreationTime(const Timestamp &newCreationTime)
{
    m_creationTime = newCreationTime;
}

CByteCounter CSnapshotInfo::getSize() const
{
    return m_size;
}

void CSnapshotInfo::setSize(const CByteCounter &newSize)
{
    m_size = newSize;
}

JSON::Object::Ptr CSnapshotInfo::toJson() const
{
	JSON::Object::Ptr obj = new JSON::Object;
	obj->set("name", m_name);
	obj->set("creationTime", m_creationTime);
	obj->set("size", m_size.getTotalBytes());
	return obj;
}

}

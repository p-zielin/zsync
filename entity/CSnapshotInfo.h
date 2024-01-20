#ifndef CSNAPSHOTINFO_H
#define CSNAPSHOTINFO_H

#include "CByteCounter.h"
#include <Poco/Timestamp.h>
#include <Poco/SharedPtr.h>
#include <Poco/JSON/Object.h>

using namespace Poco;

namespace entity
{

class CSnapshotInfo
{
public:
    typedef SharedPtr<CSnapshotInfo> Ptr;

    CSnapshotInfo();
    explicit CSnapshotInfo(const std::string & name);

    [[nodiscard]] std::string getName() const;
    void setName(const std::string &newName);

    [[nodiscard]] Timestamp getCreationTime() const;
    void setCreationTime(const Timestamp &newCreationTime);

    [[nodiscard]] CByteCounter getSize() const;
    void setSize(const CByteCounter &newSize);

	[[nodiscard]] JSON::Object::Ptr toJson() const;

protected:
    std::string m_name;
    Timestamp m_creationTime;
    CByteCounter m_size;
};

}

#endif // CSNAPSHOTINFO_H

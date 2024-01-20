#ifndef CTRANSFERSESSION_H
#define CTRANSFERSESSION_H

#include "entity/CDatasetInfo.h"

#include <string>

class CTransferSession
{
public:
    typedef Poco::SharedPtr<CTransferSession> Ptr;

    std::string name;
    std::string startSnapshot;  //! Snapshot od którego rozpocznie się przyrostowy backup.
    std::string stopSnapshot;
    bool skip = false;
    entity::CDatasetInfo::Ptr targetDataset;
};

#endif // CTRANSFERSESSION_H

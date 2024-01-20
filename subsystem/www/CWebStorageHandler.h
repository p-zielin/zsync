#ifndef CWEBSTORAGEHANDLER_H
#define CWEBSTORAGEHANDLER_H

#include "CWebHandler.h"
#include "../CZFSService.h"
#include "../backup/CBackupSubsystem.h"

#include <list>
#include <vector>

using namespace Poco;

class CWebStorageHandler : public CWebHandler
{
public:
	explicit CWebStorageHandler(const SRequestInfo& ri);

	void handleRequest(Net::HTTPServerRequest& request, Net::HTTPServerResponse& response) override;

protected:
    service::CZFSService& m_zfs;
    service::CBackupSubsystem& m_backup;
};

#endif // CWEBSTORAGEHANDLER_H

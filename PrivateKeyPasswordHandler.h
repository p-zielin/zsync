//
// Created by zielin on 06.01.24.
//

#ifndef STORAGE_PRIVATEKEYPASSWORDHANDLER_H
#define STORAGE_PRIVATEKEYPASSWORDHANDLER_H

#include <Poco/Net/PrivateKeyPassphraseHandler.h>

using namespace Poco;

class PrivateKeyPasswordHandler : public Poco::Net::PrivateKeyPassphraseHandler
{
public:
	explicit PrivateKeyPasswordHandler();

	void onPrivateKeyRequested(const void* pSender, std::string& privateKey) override;
};


#endif //STORAGE_PRIVATEKEYPASSWORDHANDLER_H

//
// Created by zielin on 06.01.24.
//

#include "PrivateKeyPasswordHandler.h"
#include <Poco/Util/Application.h>

void PrivateKeyPasswordHandler::onPrivateKeyRequested(const void* pSender, std::string& privateKey)
{
	privateKey = Util::Application::instance().config().getString("www.key_password", "");
}

PrivateKeyPasswordHandler::PrivateKeyPasswordHandler() : PrivateKeyPassphraseHandler(true)
{
}

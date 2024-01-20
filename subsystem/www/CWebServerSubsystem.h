//
// Created by zielin on 01.11.23.
//

#ifndef STORAGE_CWEBSERVERSUBSYSTEM_H
#define STORAGE_CWEBSERVERSUBSYSTEM_H

#include "CWebRequestFactory.h"

#include <Poco/Util/Subsystem.h>
#include <Poco/Util/Subsystem.h>
#include <Poco/Net/HTTPServer.h>

class CWebServerSubsystem : public Poco::Util::Subsystem
{
public:
	const char* name() const override;

	std::string getURL() const;

protected:
	void initialize(Poco::Util::Application& app) override;
	void uninitialize() override;

	Poco::SharedPtr<Poco::Net::HTTPServer> m_server;
	CWebRequestFactory::Ptr m_requestFactory;
};


#endif //STORAGE_CWEBSERVERSUBSYSTEM_H

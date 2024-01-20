//
// Created by zielin on 4/9/23.
//

#ifndef UNTITLED_WEBREQUESTFACTORY_H
#define UNTITLED_WEBREQUESTFACTORY_H

#include "CWebHandler.h"

#include <Poco/URI.h>
#include <Poco/Net/HTTPRequestHandlerFactory.h>
#include <Poco/Net/HTTPRequestHandler.h>
#include <Poco/Net/HTTPServerRequest.h>

class CWebRequestFactory : public Poco::Net::HTTPRequestHandlerFactory
{
public:
	CWebRequestFactory();
	using Ptr = Poco::SharedPtr<CWebRequestFactory>;

	virtual Poco::Net::HTTPRequestHandler* createRequestHandler(const Poco::Net::HTTPServerRequest& request) override;
	static SRequestInfo parseURL(const Poco::Net::HTTPServerRequest& request);

protected:

};

#endif //UNTITLED_WEBREQUESTFACTORY_H

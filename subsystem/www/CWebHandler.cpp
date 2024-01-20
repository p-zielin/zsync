//
// Created by zielin on 4/9/23.
//

#include "CWebHandler.h"

#include <Poco/Logger.h>
#include <Poco/Net/HTTPServerResponse.h>
#include <Poco/StringTokenizer.h>
#include <Poco/Util/Application.h>
#include <Poco/JSON/Parser.h>
#include <Poco/StreamCopier.h>

using namespace Poco;

CWebHandler::CWebHandler(const SRequestInfo& ri)
	:
	m_requestInfo(ri)
{
	Logger::root().debug("handling path: " + ri.path);
}

Poco::JSON::Object::Ptr CWebHandler::getObject(Poco::Net::HTTPServerRequest& r)
{
	std::stringstream ss;
	StreamCopier::copyStream(r.stream(), ss);
	JSON::Parser parser;
	return parser.parse(ss).extract<JSON::Object::Ptr>();
}

void CWebHandler::sendHeaders(Poco::Net::HTTPServerResponse& response)
{
	response.add("Content-Type", "application/json");
}

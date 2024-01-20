//
// Created by zielin on 4/9/23.
//

#include "CWebRequestFactory.h"
#include "CWebStorageHandler.h"

#include <Poco/Logger.h>
#include <Poco/StringTokenizer.h>
#include <Poco/DynamicStruct.h>

using namespace Poco;

CWebRequestFactory::CWebRequestFactory()
= default;

Net::HTTPRequestHandler* CWebRequestFactory::createRequestHandler(const Net::HTTPServerRequest& request)
{
	Logger::root().notice("handling request: " + request.getURI());

	SRequestInfo info = parseURL(request);
	request.response().setContentType("application/json");

	try
	{
//		if (info.controller == "storage")
		{
			return new CWebStorageHandler(info);
		}
	}
	catch (std::exception& ex)
	{
		DynamicStruct res;
		request.response().setStatus(Net::HTTPResponse::HTTP_BAD_REQUEST);
		Logger::root().error(ex.what());
		res.insert("error", ex.what());
	}

	return nullptr;
}

SRequestInfo CWebRequestFactory::parseURL(const Net::HTTPServerRequest& request)
{
	SRequestInfo ret;

	const auto uri = Poco::URI(request.getURI());

	StringTokenizer st(uri.getPath(), "/", StringTokenizer::TOK_IGNORE_EMPTY | StringTokenizer::TOK_TRIM);

	if (st.count() > 0)
	{
		ret.controller = st[0];
	}

	if (st.count() > 1)
	{
		ret.action = st[1];
	}

	for (const auto& iter : uri.getQueryParameters())
	{
		ret.query[iter.first] = iter.second;
	}

	ret.path = uri.getPath();

	return ret;
}


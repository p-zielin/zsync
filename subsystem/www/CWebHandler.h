//
// Created by zielin on 4/9/23.
//

#ifndef UNTITLED_WEBHANDLER_H
#define UNTITLED_WEBHANDLER_H

#include <Poco/Net/HTTPRequestHandler.h>
#include <Poco/Net/HTTPServerResponse.h>
#include <Poco/Net/HTTPServerRequest.h>
#include <Poco/JSON/Object.h>
#include <Poco/URI.h>

#include <map>

/*!
 * \brief Describes HTTP GET request parameters.
 */
typedef struct
{
    std::string controller;
    std::string action;
    std::string path;
    std::map<std::string, std::string> query;
}
SRequestInfo;

class CWebHandler : public Poco::Net::HTTPRequestHandler
{
public:
    explicit CWebHandler(const SRequestInfo & ri);

    static Poco::JSON::Object::Ptr getObject(Poco::Net::HTTPServerRequest &r);
    static void sendHeaders(Poco::Net::HTTPServerResponse &response);

protected:
    SRequestInfo m_requestInfo;
};


#endif //UNTITLED_WEBHANDLER_H

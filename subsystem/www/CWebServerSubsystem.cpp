//
// Created by zielin on 01.11.23.
//

#include "CWebServerSubsystem.h"
#include "PrivateKeyPasswordHandler.h"
#include <Poco/Logger.h>
#include <Poco/Util/Application.h>
#include <Poco/Net/SSLManager.h>
#include <Poco/Net/Context.h>
#include <Poco/Net/SecureServerSocket.h>

using namespace Poco;

const char* CWebServerSubsystem::name() const
{
	return "web-server";
}

void CWebServerSubsystem::initialize(Poco::Util::Application& app)
{
	Logger::root().notice("WWW server setup");
	Net::Context::Ptr ptrContext;

	const auto & addr = Net::SocketAddress(app.config().getString("www.interface", "0.0.0.0"), Util::Application::instance().config().getUInt("www.port"));
	Net::HTTPServerParams::Ptr params = new Net::HTTPServerParams();
	params->setServerName("Hosti24 WebServer");
	params->setSoftwareVersion("Hosti24 WebServer/1.0");
	m_requestFactory = new CWebRequestFactory;

	if (app.config().getBool("www.ssl", false))
	{
		const auto keyFile = app.config().getString("www.key_file");
		const auto certFile= app.config().getString("www.cert_file");
		const auto caFile = app.config().getString("www.ca_file");

		Logger::root().debug("www.key_file: '%s'", keyFile);
		Logger::root().debug("www.cert_file: '%s'", certFile);
		Logger::root().debug("www.ca_file: '%s'", caFile);

	 	ptrContext = new Net::Context(Net::Context::SERVER_USE, keyFile, certFile, caFile, Net::Context::VERIFY_RELAXED, 9, true, "TLS_AES_256_GCM_SHA384:TLS_CHACHA20_POLY1305_SHA256:TLS_AES_128_GCM_SHA256");
		Net::SSLManager::instance().initializeServer(new PrivateKeyPasswordHandler(), nullptr, ptrContext);
		m_server = new Net::HTTPServer(m_requestFactory, Net::SecureServerSocket(addr, 64, ptrContext), params);
	}
	else
	{
		m_server = new Net::HTTPServer(m_requestFactory, Net::ServerSocket(addr), params);
	}


	Logger::root().notice("WWW server listening on '%s'", addr.toString());
	m_server->start();
}

void CWebServerSubsystem::uninitialize()
{
	Logger::root().debug("Stopping WWW server");
	m_server->stop();
}

std::string CWebServerSubsystem::getURL() const
{
	return "http://" + m_server->socket().address().toString();
}

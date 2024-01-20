#include "CWebStorageHandler.h"
#include "subsystem/backup/CTransferMonitor.h"

#include <Poco/Logger.h>
#include <Poco/Util/Application.h>
#include <Poco/ThreadPool.h>
#include <Poco/JSON/Object.h>

using namespace Poco;

CWebStorageHandler::CWebStorageHandler(const SRequestInfo& ri)
	:
		CWebHandler(ri),
		m_zfs(Poco::Util::Application::instance().getSubsystem<service::CZFSService>()),
		m_backup(Poco::Util::Application::instance().getSubsystem<service::CBackupSubsystem>())
{

}

void CWebStorageHandler::handleRequest(Poco::Net::HTTPServerRequest& request, Poco::Net::HTTPServerResponse& response)
{
	Logger::root().debug("CWebStorageHandler request: '%s' - %s", m_requestInfo.path, request.getMethod());

	if (m_requestInfo.path == "/storage/list" && request.getMethod() == "GET")   //uid, mboxDir
	{
		auto& zfs = Util::Application::instance().getSubsystem<service::CZFSService>();
		DynamicStruct obj;
		const auto & name = m_requestInfo.query["name"];

		int level = 1;
		if (m_requestInfo.query.find("level") != m_requestInfo.query.end())
		{
			Poco::NumberParser::tryParse(m_requestInfo.query["level"], level);
		}

		auto ds = name.empty() ? m_zfs.getRootDataset() :m_zfs.getRootDataset()->createChildren(name);

		obj.insert("dataset", ds->getName());
		JSON::Array::Ptr arr = new JSON::Array();
		obj.insert("items", arr);

		for (auto & item : zfs.listDatasets(*ds, true, level == 0, level))
		{
			arr->add(item->toJSON());
		}

		JSON::Stringifier::stringify(obj, response.send());
	}
	else if (m_requestInfo.path == "/transfer/list" && request.getMethod() == "GET")
	{
		JSON::Stringifier::stringify(CTransferMonitor::instance()->toJSON(), response.send());
	}
	else if (m_requestInfo.path == "/transfer/clear" && request.getMethod() == "PUT")
	{
		CTransferMonitor::instance()->clear();
	}
	else if (m_requestInfo.path == "/backup/removeOld" && request.getMethod() == "PUT")
	{
        DynamicStruct ds;
        ds.insert("success", m_backup.removeOldBackups());
        JSON::Stringifier::stringify(ds, response.send());
	}
	else if (m_requestInfo.path == "/backup/abort" && request.getMethod() == "PUT")
	{
		m_backup.stopTransfers();
	}
	else if (m_requestInfo.path == "/backup/run" && request.getMethod() == "PUT")
	{
		Logger::root().debug("Starting backup");

		if (m_backup.getMode() != service::CBackupSubsystem::Mode::master)
		{
			throw Poco::Exception("Program is not working in master mode");
		}

		if (!m_backup.tryRun())
		{
            DynamicStruct qs;
            qs.insert("message", "Backup is already running");
            response.setStatus(Poco::Net::HTTPResponse::HTTP_NOT_ACCEPTABLE);
            JSON::Stringifier::stringify(qs, response.send());
			Logger::root().error("Backup is already running");
            return;
		}
	}
	else if (m_requestInfo.path == "/backup/queue" && request.getMethod() == "GET")
	{
		DynamicStruct qs;
		qs.insert("queueSize", m_backup.getQueueSize());
		JSON::Stringifier::stringify(qs, response.send());
	}

	response.send();
}

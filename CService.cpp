//
// Created by zielin on 01.11.23.
//

#include "CService.h"
#include "subsystem/www/CWebServerSubsystem.h"
#include "subsystem/CLoggingSubsystem.h"
#include "subsystem/CShellSubsystem.h"
#include "subsystem/CZFSService.h"
#include "subsystem/backup/CBackupSubsystem.h"

#include <Poco/Util/HelpFormatter.h>
#include <Poco/Util/OptionCallback.h>
#include <Poco/Util/Option.h>
#include <Poco/Util/IniFileConfiguration.h>
#include "subsystem/backup/workers/CCleanerWorker.h"

using namespace Poco;

CService::CService()
{
    addSubsystem(new CLoggingSubsystem);
    addSubsystem(new service::CZFSService);
    addSubsystem(new CShellSubsystem);
}

const char* CService::name() const
{
	return "storage";
}

void CService::initialize(Poco::Util::Application& self)
{
	if (!m_configLoaded)
	{
		handleConfig("config", "storage.ini");
	}

	if (self.config().getBool("remove-old", false))
	{
		Logger::root().information("Onetime start - remove overtimed");
		addSubsystem(new service::CBackupSubsystem);
	}
    else if (self.config().getBool("backup.once", false))
    {
        self.config().setBool("daemon", false);
        Logger::root().information("Onetime start - backup");
        addSubsystem(new service::CBackupSubsystem);
    }
    else
    {
        self.config().setBool("daemon", true);
        addSubsystem(new service::CBackupSubsystem);
        addSubsystem(new CWebServerSubsystem);
    }

	Application::initialize(self);
    Logger::root().debug("Initialization finished");
}

void CService::uninitialize()
{
	Application::uninitialize();
}

void CService::reinitialize(Poco::Util::Application& self)
{
	Application::reinitialize(self);
}

int CService::main(const std::vector<std::string>& args)
{
	Logger::root().debug("main function");

    if (config().getBool("backup.once", false))
    {
        getSubsystem<service::CBackupSubsystem>().run();
        return Util::Application::EXIT_OK;
    }

	if (config().getBool("remove-old", false))
	{
		if (CCleanerWorker::instance()->start())
        {
            CCleanerWorker::instance()->join();
        }
		return Util::Application::EXIT_OK;
	}

    if (config().getBool("daemon", true))
    {
        Logger::root().information("Service mode");
        waitForTerminationRequest();
    }

    Logger::root().notice("Finishing");
	return Util::Application::EXIT_OK;
}

void CService::defineOptions(Poco::Util::OptionSet &options)
{
    Poco::Util::ServerApplication::defineOptions(options);

    options.addOption(
        Util::Option("help", "h", "displays help")
            .required(false)
            .repeatable(false)
            .callback(Util::OptionCallback<CService>(this, &CService::handleHelp)));

    options.addOption(
        Util::Option("config", "", "path to config file")
            .required(false)
            .repeatable(false)
            .argument("file")
            .callback(Util::OptionCallback<CService>(this, &CService::handleConfig)));

    options.addOption(
        Util::Option("send-backup", "", "onetime start and send backup")
            .required(false)
            .repeatable(false)
            .callback(Util::OptionCallback<CService>(this, &CService::handleConfig)));

	options.addOption(
		Util::Option("remove-old", "", "remove overtimed snapshots")
			.required(false)
			.repeatable(false)
			.callback(Util::OptionCallback<CService>(this, &CService::handleConfig)));

    options.addOption(
        Util::Option("rebuild-users", "", "Odbuduj konta i grupy uzytkownikow")
            .required(false)
            .repeatable(false)
            .callback(Util::OptionCallback<CService>(this, &CService::handleConfig)));

    options.addOption(
        Util::Option("rebuild-mboxes", "", "Odbuduj skrzynki pocztowe")
            .required(false)
            .repeatable(false)
            .callback(Util::OptionCallback<CService>(this, &CService::handleConfig)));

    options.addOption(
        Util::Option("rebuild-services", "", "Odbuduj datasety i katalogi uslug")
            .required(false)
            .repeatable(false)
            .callback(Util::OptionCallback<CService>(this, &CService::handleConfig)));

    options.addOption(
        Util::Option("register", "", "Zarejestruj macierz w portalu")
            .required(false)
            .repeatable(false)
            .callback(Util::OptionCallback<CService>(this, &CService::handleConfig)));}

void CService::handleConfig(const std::string &name, const std::string &value)
{
    Logger::root().notice(name + "   " + value);

    if (name == "config")
    {
        config().add(new Util::IniFileConfiguration(value));
		m_configLoaded = true;
    }
    else if (name == "send-backup")
    {
        config().setBool("backup.once", true);
    }
	else if (name == "send-backup")
	{
		config().setBool("remove-old", true);
	}
}

void CService::handleHelp(const std::string &name, const std::string &value)
{
    Util::HelpFormatter helpFormatter(options());
    helpFormatter.setCommand(commandName());
    helpFormatter.setHeader("storage - application for sending ZFS backups");

    helpFormatter.format(std::cout);
    exit(0);
}

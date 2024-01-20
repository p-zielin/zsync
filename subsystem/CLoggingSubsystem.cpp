//
// Created by zielin on 4/9/23.
//

#include "CLoggingSubsystem.h"

#include <Poco/Logger.h>
#include <Poco/ConsoleChannel.h>
#include <Poco/AsyncChannel.h>
#include <Poco/SplitterChannel.h>
#include <Poco/FormattingChannel.h>
#include <Poco/PatternFormatter.h>
#include <Poco/Util/Application.h>
#include <Poco/File.h>
#include <Poco/FileChannel.h>

const char* CLoggingSubsystem::name() const
{
	return "Logging";
}

void CLoggingSubsystem::initialize(Poco::Util::Application& app)
{
	AutoPtr<ColorConsoleChannel> pCons;
    AutoPtr<SplitterChannel> pSplitter(new SplitterChannel);
	if (app.config().getBool("log.async", false))
	{
		AutoPtr<AsyncChannel> pAsync = new AsyncChannel;
		pAsync->setChannel(pSplitter);
		Logger::root().setChannel(pAsync);
	}
	else
	{
		Logger::root().setChannel(pSplitter);
	}

    if (app.config().getBool("log.console", true))
    {
        pCons = new ColorConsoleChannel;

        if (app.config().getBool("log.colors", true))
        {
            pCons->setProperty("enableColors", "true");
            pCons->setProperty("criticalColor", "lightRed");
            pCons->setProperty("warningColor", "yellow");
            pCons->setProperty("fatalColor", "lightRed");
            pCons->setProperty("errorColor", "lightRed");
            pCons->setProperty("informationColor", "lightGreen");
        }

        AutoPtr<FormattingChannel> pFC(new FormattingChannel(new PatternFormatter("%L%Y-%m-%d %H:%M:%S [%p]: %t"), pCons));
        pSplitter->addChannel(pFC);
    }

    auto pFile = createFileLogger(app);

    if (!pFile.isNull())
    {
        pFile->setProperty("flush", app.config().getString("log.flush", "true"));
        AutoPtr<FormattingChannel> pFC(new FormattingChannel(new PatternFormatter("%L%Y-%m-%d %H:%M:%S [%p]: %t"), pFile));
        pSplitter->addChannel(pFC);
    }

    Logger::root().setLevel(app.config().getString("log.level", "notice"));
    Logger::root().debug("Logger ready");
}

void CLoggingSubsystem::uninitialize()
{
    Logger::root().debug("Logger shutdown");
    Logger::shutdown();
}

void CLoggingSubsystem::reinitialize(Poco::Util::Application& app)
{
    Logger::root().debug("Logger reinitialize");
    Subsystem::reinitialize(app);
}

AutoPtr<FileChannel> CLoggingSubsystem::createFileLogger(const Util::Application& app)
{
    if (!app.config().getBool("file", true))
    {
        return nullptr;
    }

    Poco::Path path(app.config().getString("log.dir", "./log"));

    if (path.isRelative())
    {
        path = path.makeAbsolute();
    }

    const std::string logFile = path.toString() + "/storage.log";
    Poco::File file(path);
    file.createDirectories();

    AutoPtr<FileChannel> pFile(new FileChannel(logFile));

    if (app.config().has("log.rotation"))
    {
        pFile->setProperty("rotation", app.config().getString("log.rotation", "daily"));
        pFile->setProperty("compress", app.config().getString("log.compress", "true"));
        pFile->setProperty("rotateOnOpen", app.config().getString("log.rotate_open", "true"));
    }

    return pFile;
}

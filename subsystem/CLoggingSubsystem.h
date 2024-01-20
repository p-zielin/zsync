//
// Created by zielin on 4/9/23.
//

#pragma once
#include <Poco/Util/Subsystem.h>
#include <Poco/Util/Application.h>
#include <Poco/FileChannel.h>

using namespace Poco;

class CLoggingSubsystem : public Poco::Util::Subsystem
{
public:
	const char* name() const override;

protected:
	void initialize(Poco::Util::Application& app) override;
	void uninitialize() override;
	void reinitialize(Poco::Util::Application& app) override;
    static AutoPtr<Poco::FileChannel> createFileLogger(const Util::Application& app);
};

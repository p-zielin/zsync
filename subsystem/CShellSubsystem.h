//
// Created by zielin on 01.11.23.
//

#ifndef STORAGE_CSHELLSUBSYSTEM_H
#define STORAGE_CSHELLSUBSYSTEM_H

#include <Poco/Util/Subsystem.h>
#include <Poco/Process.h>

class CShellSubsystem : public Poco::Util::Subsystem
{
public:
	const char* name() const override;

	static std::vector<std::string> getOutputLines(const std::string& cmd, const Poco::Process::Args& args);

	static void check(const std::string& shellValue);

protected:
	void initialize(Poco::Util::Application& app) override;
	void uninitialize() override;
};


#endif //STORAGE_CSHELLSUBSYSTEM_H

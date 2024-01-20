//
// Created by zielin on 01.11.23.
//

#include "CShellSubsystem.h"

#include <Poco/Logger.h>
#include <Poco/PipeStream.h>
#include <Poco/String.h>

using namespace Poco;

const char* CShellSubsystem::name() const
{
	return "shel-subsystem";
}

void CShellSubsystem::initialize(Poco::Util::Application& app)
{
}

void CShellSubsystem::uninitialize()
{

}

std::vector<std::string> CShellSubsystem::getOutputLines(const std::string& cmd, const Poco::Process::Args& args)
{
	std::vector<std::string> ret;
	Poco::Pipe outPipe;
    Poco::Pipe errPipe;
	Poco::PipeInputStream istr(outPipe);

	check(cmd);

	for (const auto& arg : args)
	{
		check(arg);
	}

	Logger::root().debug(cmd +  " " + cat<std::string>(" ", args.begin(), args.end()));
    Poco::ProcessHandle ph(Process::launch(cmd, args, nullptr, &outPipe, &errPipe));

	while (ph.tryWait() == -1)
	{
		std::string line;

		while (std::getline(istr, line))
		{
			ret.push_back(line);
		}
	}

	return ret;
}

void CShellSubsystem::check(const std::string& shellValue)
{
	if (shellValue.find('&') != std::string::npos)
	{
		throw Poco::Exception("Niedozwolony znak");
	}

	if (shellValue.find('|') != std::string::npos)
	{
		throw Poco::Exception("Niedozwolony znak");
	}
}

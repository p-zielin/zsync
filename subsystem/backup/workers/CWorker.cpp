//
// Created by zielin on 21.12.23.
//

#include "CWorker.h"

#include <Poco/Process.h>
#include <Poco/Logger.h>
#include <sys/wait.h>

void CWorker::printZFSError(PipeInputStream& in)
{
	std::string line;
	while (std::getline(in, line))
	{
		Logger::root().warning(line);
	}
}

void CWorker::killZFS(const ProcessHandle& handle)
{
	if (Process::tryWait(handle) == -1)
	{
		Logger::root().information("Finishing ZFS process - PID '%s'", std::to_string((int)handle.id()));
        Process::kill(handle.id());
		Process::wait(handle);
	}
}

void CWorker::abort()
{
	m_abort = true;
}

void CWorker::setSocketTimeout(int socketTimeout)
{
	m_socketTimeout = std::max<int>(5, socketTimeout);
}

void CWorker::setBufferSize(int size)
{
	if (size < 1)
	{
		throw Poco::Exception("Buffer size can't be < 1");
	}

	m_bufferSize = std::max<int>(1, size);
}

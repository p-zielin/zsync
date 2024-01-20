//
// Created by zielin on 21.12.23.
//

#ifndef STORAGE_CWORKER_H
#define STORAGE_CWORKER_H

#include <unistd.h>
#include <Poco/PipeStream.h>
#include <Poco/Net/TCPServerConnection.h>
#include <Poco/Process.h>

using namespace Poco;

class CWorker
{
public:
	static void killZFS(const ProcessHandle& pid);
	static void printZFSError(Poco::PipeInputStream& in);
	virtual void abort();

	void setSocketTimeout(int socketTimeout);
	void setBufferSize(int size);

protected:
	bool m_abort = false;
	int m_socketTimeout = 10;
	int m_bufferSize = 1;
};


#endif //STORAGE_CWORKER_H

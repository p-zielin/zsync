//
// Created by zielin on 19.11.23.
//

#ifndef STORAGE_CCLEANERWORKER_H
#define STORAGE_CCLEANERWORKER_H

#include <Poco/Thread.h>
#include <Poco/Runnable.h>

using namespace Poco;

class CCleanerWorker : public Runnable
{
public:
    explicit CCleanerWorker();
    void run() override;
    static CCleanerWorker *instance();
    bool start();
    bool isRunning() const;
    void join();

protected:
    static Thread m_thread;
};


#endif //STORAGE_CCLEANERWORKER_H

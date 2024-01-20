#ifndef CTRANSFERMONITOR_H
#define CTRANSFERMONITOR_H

#include "CTransferNotification.h"
#include "CTransferInfo.h"

#include <Poco/NotificationCenter.h>
#include <Poco/NotificationQueue.h>
#include <Poco/SingletonHolder.h>
#include <Poco/Mutex.h>
#include <Poco/Thread.h>
#include <map>

using namespace Poco;

/*!
 * \brief Obiekt zbierajacy powiadomienia z watkow nadajacych/odbierajacych. Z drugiej strony sa obserwerzy, ktorzy dostaja powiadomienia o aktualnym stanie.
 */
class CTransferMonitor : public Runnable
{
public:
    static CTransferMonitor* instance();
    ~CTransferMonitor() override;

    /*!
     * \brief Czyści zakończone transfery.
     */
    void clear();

    void run() override;
    /*!
     * \brief Wysyla powiadomienie do kolejki. W zaleznosci od potrzeby zaklada ono wpis zw. z datasetem, usuwa go lub aktualizuje.
     * Automatycznie powiadamiani sa obserwatorzy podlaczeni do CTransferMonitor poprzez \a m_center.
     * \param notification
     */
    void send(CTransferNotification *notification);

	/*!
	 * \brief Pozwala wyświetlić transfery filtrując ich status.
	 * @param os Strumień wyjściowy
	 * @param state
	 */
    void getTransfers(std::ostream & os, CTransferInfo::State state = CTransferInfo::State::any);
	JSON::Object::Ptr toJSON();
	void stop();
protected:
    CTransferMonitor();

    FastMutex m_mtx;
    std::map<std::string, CTransferInfo::Ptr> m_transfers;
    NotificationQueue m_queue;
    NotificationCenter m_center;
    Thread m_thread;
    friend class Poco::SingletonHolder<CTransferMonitor>;
};


#endif // CTRANSFERMONITOR_H

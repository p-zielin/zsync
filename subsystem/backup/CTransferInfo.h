#ifndef CTRANSFERINFO_H
#define CTRANSFERINFO_H

#include "CByteCounter.h"
#include "entity/CDatasetInfo.h"

#include <Poco/LocalDateTime.h>
#include <string>
#include <list>

using namespace Poco;

/*!
 * \brief Obiekt opisujący transfer datasetu.
 */
class CTransferInfo : public CBaseEntity
{
public:
    enum class State : int
    {
		any,
        waiting,
        running,
        finished
    };

    typedef SharedPtr<CTransferInfo> Ptr;
    CTransferInfo();

	[[nodiscard]] JSON::Object::Ptr toJSON() const override;

	void setRetry(bool retry);
	void setId(const std::string & id);
	void setDataset(const entity::CDatasetInfo::Ptr & ds);
	void setStartTime(const Timestamp & timestamp);
	void setLastUpdate(const Timestamp & timestamp);
	void setStartSnapshot(const std::string & snapshot);
	void setStopSnapshot(const std::string & snapshot);
	void setState(State state);
	void setCounter(const CByteCounter& counter);
	void addMessage(const std::string & msg);
	void setStopTime(const Timestamp & timestamp);

	[[nodiscard]] std::string getId() const;
	[[nodiscard]] bool isRetry() const;
	[[nodiscard]] State getState() const;
	[[nodiscard]] std::string getDatasetName() const;
	[[nodiscard]] CByteCounter getCounter() const;
	[[nodiscard]] Timestamp getStartTime() const;
	[[nodiscard]] Timestamp getStopTime() const;
	[[nodiscard]] Timestamp getLastUpdate() const;

protected:
	entity::CDatasetInfo::Ptr m_dataset;
	CByteCounter m_counter;
	std::list<std::string> m_messages;
	State m_state;
};

#endif // CTRANSFERINFO_H

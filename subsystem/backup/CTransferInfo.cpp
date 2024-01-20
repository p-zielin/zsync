#include "CTransferInfo.h"

#include <Poco/JSON/Object.h>
#include <Poco/JSON/Stringifier.h>
#include <Poco/JSON/Array.h>

using namespace Poco;

CTransferInfo::CTransferInfo()
= default;

JSON::Object::Ptr CTransferInfo::toJSON() const
{
	JSON::Object::Ptr obj = CBaseEntity::toJSON();

	switch (m_state)
	{
	case State::any:
		obj->set("state", "any");
		break;
	case State::waiting:
		obj->set("state", "waiting");
		break;
	case State::running:
		obj->set("state", "running");
		break;
	case State::finished:
		obj->set("state", "finished");
		break;
	}

    obj->set("dataset", m_dataset->toJSON());

	if (!m_messages.empty())
	{
		JSON::Array::Ptr arr = new JSON::Array;

		for (const auto & msg : m_messages)
		{
			arr->add(msg);
		}

		obj->set("messsages", arr);
	}

	JSON::Object::Ptr counter = new JSON::Object;
	counter->set("counter", m_counter.toString());
	counter->set("totalBytes", m_counter.getTotalBytes());
    obj->set("counter", counter);
	return obj;
}

void CTransferInfo::setRetry(bool retry)
{
	erase("retry");
	insert("retry", retry);
}

bool CTransferInfo::isRetry() const
{
	return getVar("retry", false);
}

void CTransferInfo::setId(const std::string & id)
{
	erase("id");
	insert("id", id);
}

std::string CTransferInfo::getId() const
{
	return getVar("id", "");
}

void CTransferInfo::setDataset(const entity::CDatasetInfo::Ptr& ds)
{
	m_dataset = ds;
}

void CTransferInfo::setStartTime(const Timestamp & timestamp)
{
	erase("startTime");
	insert("startTime", timestamp);
}

void CTransferInfo::setLastUpdate(const Timestamp & timestamp)
{
	erase("lastUpdate");
	insert("lastUpdate", timestamp);
}

void CTransferInfo::setStartSnapshot(const std::string & snapshot)
{
	erase("startSnapshot");
	insert("startSnapshot", snapshot);
}

void CTransferInfo::setStopSnapshot(const std::string & snapshot)
{
	erase("stopSnapshot");
	insert("stopSnapshot", snapshot);
}

void CTransferInfo::setState(CTransferInfo::State state)
{
	m_state = state;
}

void CTransferInfo::setCounter(const CByteCounter& counter)
{
	m_counter = counter;
}

void CTransferInfo::addMessage(const std::string& msg)
{
	m_messages.push_back(msg);
}

void CTransferInfo::setStopTime(const Timestamp& timestamp)
{
	erase("stopTime");
	insert("stopTime", timestamp);
}

CTransferInfo::State CTransferInfo::getState() const
{
	return m_state;
}

std::string CTransferInfo::getDatasetName() const
{
	return m_dataset.isNull() ? "" : m_dataset->getName();
}

CByteCounter CTransferInfo::getCounter() const
{
	return m_counter;
}

Timestamp CTransferInfo::getStartTime() const
{
	return getVar("startTime", 0);
}

Timestamp CTransferInfo::getLastUpdate() const
{
	return getVar("lastUpdate", 0);
}

Timestamp CTransferInfo::getStopTime() const
{
	return getVar("stopTime", 0);
}

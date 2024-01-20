//
// Created by zielin on 11/2/23.
//

#pragma once

#include "CBaseEntity.h"
#include <string>

class CByteCounter;

namespace entity
{

/*!
 * \brief Obiekt określający usługę klienta z perspektywy storage.
 */
	class CUserService : public CBaseEntity
	{
	public:

		typedef Poco::SharedPtr<CUserService> Ptr;
		[[nodiscard]] int getUID() const;
		[[nodiscard]] int getGID() const;
		[[nodiscard]] unsigned long getId() const;
		[[nodiscard]] std::string getInstanceName() const;
		[[nodiscard]] std::string getName() const;
		[[nodiscard]] std::string getDescription() const;
		[[nodiscard]] bool isCompleted() const;
		[[nodiscard]] int getDataLayout() const;
		///! Wyliczane na podstawie dataLayout oraz diskSpace wartość quoty wyrażona w GB
		[[nodiscard]] CByteCounter getCalculatedQuota() const;
		[[nodiscard]] int getDiskSpace() const;
		[[nodiscard]] bool getSnaphots() const;
	};

}

typedef std::vector<entity::CUserService::Ptr> ServiceList;
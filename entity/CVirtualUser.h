#ifndef CVIRTUALUSER_H
#define CVIRTUALUSER_H

#include "CBaseEntity.h"
#include "CUserService.h"

namespace entity
{

	class CVirtualUser : public CBaseEntity
	{
	public:
		typedef Poco::SharedPtr<CVirtualUser> Ptr;
		CVirtualUser();

		void fromJSON(const JSON::Object::Ptr& obj) override;

		[[nodiscard]] std::string getName() const;
		[[nodiscard]] std::string getMboxDir() const;

		[[nodiscard]] CUserService::Ptr getUserService() const;
		void setUserService(const CUserService::Ptr & us);

		void setChmod(int i);
		[[nodiscard]] int getChmod() const;
		[[nodiscard]] int getUID() const;

		[[nodiscard]] unsigned long getQUotaLimitBytes() const;
		[[nodiscard]] bool isResponderEnabled() const;
		[[nodiscard]] bool isRedirectEnabled() const;
		[[nodiscard]] bool isLeaveCopyEnabled() const;
		[[nodiscard]] bool isDeleteSpam() const;
		[[nodiscard]] bool isAllIntoMailbox() const;
		[[nodiscard]] bool isSnapshot() const;
		[[nodiscard]] std::string getResponderBody() const;
		[[nodiscard]] std::string getResponderSubject() const;

		[[nodiscard]] std::string getSnapshotName() const;

	protected:
		CUserService::Ptr m_service;
		int m_chmod = 0;
	};

}
#endif // CVIRTUALUSER_H

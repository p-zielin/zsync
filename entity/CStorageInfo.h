#ifndef CSTORAGE_H
#define CSTORAGE_H

#include "CBaseEntity.h"
#include "CDatasetInfo.h"

namespace entity
{
/*!
 * \brief Obiekt opisujący macierz, na której został uruchomiony program.
 */
	class CStorageInfo : public CBaseEntity
	{
	public:
		typedef Poco::SharedPtr<CStorageInfo> Ptr;

		CStorageInfo();
		void setDatasetInfo(CDatasetInfo::Ptr dataset);

		[[nodiscard]] unsigned long getId() const;
		[[nodiscard]] std::string getName() const;
		[[nodiscard]] bool getOnlyLocal() const;
		[[nodiscard]] std::string getRestUrl() const;
		[[nodiscard]] std::string getMountPath() const;
		void setMountPath(const std::string& path);
		[[nodiscard]] int getTotalSpace() const;
		[[nodiscard]] int getUsedSpace() const;
		void setId(unsigned long id);
		void setName(const std::string& name);
		void setRestUrl(const std::string& url);
		[[nodiscard]] std::string getDataset() const;
		void setDataset(const std::string& dataset);
		void setJailGID(uint gid);
		[[nodiscard]] int getJailGID() const;
		void setMailGID(uint gid);
		[[nodiscard]] uint getMailGID() const;
		[[nodiscard]] int getMailChmod() const;
		void setMailChmod(int newMailChmod);
		[[nodiscard]] int getJailChmod() const;
		void setJailChmod(int jailChmod);
		[[nodiscard]] int getParentChmod() const;
		void setParentChmod(int newParentChmod);
		void setNetworkAccess(const std::string& address);
		[[nodiscard]] std::string getNetworkAccess() const;
		[[nodiscard]] int getWwwUID() const;
		void setWwwUID(int newWwwUID);

	protected:
		int m_mailChmod = 0;
		int m_jailChmod = 0;
		int m_parentChmod = 0;
		int m_wwwUID = 80;
	};
}

#endif // CSTORAGE_H

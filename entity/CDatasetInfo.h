//
// Created by zielin on 01.11.23.
//

#ifndef STORAGE_CDATASETINFO_H
#define STORAGE_CDATASETINFO_H

#include "CBaseEntity.h"
#include "CSnapshotInfo.h"
#include "CByteCounter.h"

#include <string>
#include <sys/types.h>
#include <Poco/JSON/Object.h>

namespace entity
{
	class CDatasetInfo : public CBaseEntity
	{
	public:
		typedef Poco::SharedPtr<CDatasetInfo> Ptr;

        explicit CDatasetInfo(const std::string& path = "");
		explicit CDatasetInfo(const JSON::Object::Ptr & json);
		CDatasetInfo(const CDatasetInfo& di);

		[[nodiscard]] uint getUID() const;
		[[nodiscard]] uint getGID() const;

		[[nodiscard]] std::string getName() const;
		[[nodiscard]] CByteCounter getUsage() const;
		[[nodiscard]] CByteCounter getQuota() const;
		[[nodiscard]] CByteCounter getRefQuota() const;
		[[nodiscard]] CByteCounter getAvail() const;
		[[nodiscard]] uint getChmod() const;
		[[nodiscard]] std::string getMountPoint() const;
		[[nodiscard]] std::string getShareNFS() const;

		void setChmod(uint chmod);
		void setMountPoint(const std::string& newMountPoint);
		void setQuota(unsigned long long quota);
		void setQuota(const CByteCounter & newQuota);
		void setName(const std::string& name);
		void setUID(uint uid);
		void setGID(uint gid);
		void setShareNFS(const std::string& shareNFS);
		static CDatasetInfo::Ptr fromZfsLine(const std::string& zfsLine);
        void setRelativeName(const std::string & parentName);
		void setCreationTime(const Timestamp & creationTime);

		/*!
		 * \brief Tworzy podrzędny dataset, który posiada te same atrybuty co rodzic.
		 * \param name Nazwa bez wiodącego znaku '/'
		 * \return wskaźnik na nowy obiekt.
		 */
		[[nodiscard]] CDatasetInfo::Ptr createChildren(const std::string& name) const;

        [[nodiscard]] std::vector<CSnapshotInfo::Ptr> getSnapshots() const;
        void setSnapshots(const std::vector<entity::CSnapshotInfo::Ptr>& snaps);
        void appendSnapshot(const entity::CSnapshotInfo::Ptr & snapshot);
		[[nodiscard]] std::string getRelativeName() const;
		[[nodiscard]] Timestamp getCreationTime() const;
		[[nodiscard]] JSON::Object::Ptr toJSON() const override;

		/*~
		 * \brief Zwraca nzwy migawek, które następują po ostatnim backupie.
		 * \param backupPrefix - prefix dla backupów - po natknięciu się na snapshot zczynający się od tej zmiennej,
		 * przeszukiwanie kończy się i zwracane jest to co udało się zgromadzić.
		 */
		[[nodiscard]] std::vector<std::string> getFirstAvailableSnapshot(const std::string& backupPrefix) const;

	protected:
        CByteCounter m_quota;
        CByteCounter m_refquota;
        CByteCounter m_used;
        CByteCounter m_avail;
        std::vector<CSnapshotInfo::Ptr> m_snapshots;
	};

}

#endif //STORAGE_CDATASETINFO_H

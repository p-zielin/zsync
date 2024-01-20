//
// Created by zielin on 01.11.23.
//

#ifndef STORAGE_CZFSSERVICE_H
#define STORAGE_CZFSSERVICE_H

#include <Poco/Util/Subsystem.h>
#include <vector>

#include "../entity/CDatasetInfo.h"

namespace service {
/*!
 * \brief Klasa odpowiedzialna za obsługę storage ZFS. Pozwala na zarządzanie dataasetami.
 */
class CZFSService : public Poco::Util::Subsystem
{
public:
    const char *name() const override;

	/*!
	 * \brief Wykonuje rollback do określonego snapshotu
	 * @param ds Dataset, który ma być cofnięty.
	 * @param snapshot Nazwa snapshotu
	 * @return Zwraca true, jeśli udało się wykonać poprawnie komendę.
	 */
	static bool rollback(const entity::CDatasetInfo::Ptr & ds, const std::string & snapshot);

    /*!
     * \brief Listuje datasaety i zwraca ich najważniejsze parametry.
     * \param rootDataset Nadrzędny dataset. Jeśli nie jest określony, brane są pod uwagę wszystkie zbiory.
     * \param includeSnapshots Określa czy mają być pokazane jedynie snapshoty
     * \param recursive Rekursywnie
     * \return Lista CDatasetInfo - obiekt opisujący dataset.
     */
    std::vector<entity::CDatasetInfo::Ptr> listDatasets(const entity::CDatasetInfo & rootDataset, bool includeSnapshots = false, bool recursive = true, int level = 0);

    /*!
     * \beief Zwraca informacje o konkretnym datasecie.
     * \param name Nazwa datasetu
     * \return obiekt opisujący dataset
     */
    entity::CDatasetInfo::Ptr listDataset(const std::string &name, bool includeSnapshots = false);

    static bool removeSnapshots(const entity::CDatasetInfo& dataset, const std::string & from, const std::string & to, bool recursive = false);
    static bool removeSnapshot(const entity::CDatasetInfo& dataset, const std::string & name, bool recursive = false);
    /*!
     * \brief Tworzy nowy dataset.
     * \param dataset Parametry datasetu (nazwa, quota, uid).
     * @return bool True gdy się powiedzie seria wywołań. W przypadku błędu dataset jest usuwany.
     */
    bool createDataset(entity::CDatasetInfo::Ptr dataset);

    bool destroyDataset(entity::CDatasetInfo::Ptr dataset);

    /*!
     * \brief Wykonuje chmod oraz chown na datasecie.
     * \param dataset
     */
    static void restoreACL(entity::CDatasetInfo::Ptr dataset);

	static void setReadonly(const entity::CDatasetInfo& ds, bool ro);

	/*!
	 * \brief Przywraca ustawienia konkretnego datasetu usługi.
	 * Ustawiany jest UID katalogów, quota, sharenfs.
	 * @param ds Dataset usługi.
	 * @return true jeśli uda się zmodyfikować dataset.
	 */
    static bool restoreSettings(entity::CDatasetInfo::Ptr ds);

    static bool createDatasetByName(const std::string & name);

    static void printLastError();

    static bool setQuota(const entity::CDatasetInfo::Ptr & ds);

    static void getStat(entity::CDatasetInfo& ds);

    static bool createSnapshot(const entity::CDatasetInfo & ds, const std::string &snapshot, bool recursive = false);

    entity::CDatasetInfo::Ptr getRootDataset() const;

	static void attachSnapshots(entity::CDatasetInfo::Ptr& ds, const std::string & prefixFilter = "");

protected:
    static const  std::string m_zfsColumns;
    void initialize(Poco::Util::Application &app) override;
    void uninitialize() override;
    entity::CDatasetInfo::Ptr m_rootDataset;
};

}

#endif //STORAGE_CZFSSERVICE_H

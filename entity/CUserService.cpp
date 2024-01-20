//
// Created by zielin on 11/2/23.
//

#include "CUserService.h"
#include "CByteCounter.h"

namespace entity
{

	int CUserService::getUID() const
	{
		return getVar("uid", -1);
	}

	int CUserService::getGID() const
	{
		return getVar("gid", -1);
	}

	unsigned long CUserService::getId() const
	{
		return getVar("id");
	}

	std::string CUserService::getInstanceName() const
	{
		return getVar("instanceName");
	}

	int CUserService::getDiskSpace() const
	{
		return getVar("diskSpace", 0);
	}

	bool CUserService::getSnaphots() const
	{
		return getVar("snapshots", true);
	}

	std::string CUserService::getName() const
	{
		return getVar("name", "");
	}

	std::string CUserService::getDescription() const
	{
		return getVar("description", "");
	}

	bool CUserService::isCompleted() const
	{
		return getVar("state", "") == "Completed";
	}

	int CUserService::getDataLayout() const
	{
		return getVar("dataLayout", 100);
	}

	CByteCounter CUserService::getCalculatedQuota() const
	{
		CByteCounter counter;
		auto diskSpace = getDiskSpace();

		if (diskSpace == 0)
		{
			return counter;
		}

		auto layout = getDataLayout();
		auto gb = (float) (diskSpace * layout) / 100;
		counter.addGBytes(gb);
		counter.addMBytes( (gb - (int) gb) * 1000);
		return counter;
	}

}

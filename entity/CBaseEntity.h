//
// Created by zielin on 01.11.23.
//

#ifndef STORAGE_CBASEENTITY_H
#define STORAGE_CBASEENTITY_H

#include <istream>

#include <Poco/SharedPtr.h>
#include <Poco/DynamicStruct.h>
#include <Poco/JSON/Object.h>

using namespace Poco;

class CBaseEntity : public DynamicStruct
{
public:
	typedef SharedPtr<CBaseEntity> Ptr;
	void clone(const CBaseEntity& e);

	virtual void fromJSON(const JSON::Object::Ptr& obj);
	virtual void fromStream(std::istream& is);

	[[nodiscard]] virtual JSON::Object::Ptr toJSON() const;

	template<class T> static std::list<DynamicStruct> toList(const std::vector<T>& entites)
	{
		std::list<DynamicStruct> ret;

		for (auto i = 0; i < entites.size(); ++i)
		{
			ret.push_back(*entites[i]);
		}

		return ret;
	}

};


#endif //STORAGE_CBASEENTITY_H

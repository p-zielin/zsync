//
// Created by zielin on 01.11.23.
//

#include "CBaseEntity.h"
#include <Poco/JSON/Parser.h>
#include <list>
#include <Poco/Logger.h>

void CBaseEntity::clone(const CBaseEntity& e)
{
	for (const auto & iter : e)
	{
		erase(iter.first);
		insert(iter.first, iter.second);
	}
}

void CBaseEntity::fromJSON(const JSON::Object::Ptr& obj)
{
	for (const auto & iter : *obj)
	{
		Logger::root().debug("%s = %s", iter.first, iter.second.toString());
		insert(iter.first, iter.second);
	}
}

void CBaseEntity::fromStream(std::istream& is)
{
	clear();
	JSON::Parser parser;
	fromJSON(parser.parse(is).extract<JSON::Object::Ptr>());
}

JSON::Object::Ptr CBaseEntity::toJSON() const
{
	JSON::Object::Ptr obj = new JSON::Object;

	for (auto & iter : *this)
	{
		if (iter.second.isArray())
		{
			JSON::Array::Ptr arr = new JSON::Array();
			obj->set(iter.first, arr);
		}
		else
		{
			obj->set(iter.first, iter.second);
		}
	}

	return obj;
}

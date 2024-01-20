//
// Created by zielin on 01.11.23.
//

#include "CHelper.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <Poco/StringTokenizer.h>
#include <Poco/NumberParser.h>

using namespace Poco;

unsigned long long CHelper::stringToKBytes(const std::string& str)
{
	if (str.empty() || str == "-")
	{
		return 0;
	}

    const auto & sub = str.substr(0, str.size() - 1);

    if (sub.empty())
    {
        return 0;
    }

    unsigned long long value = std::stoull(sub);
	auto last = str.size() - 1;

	switch (str[last])
	{
	case 'M':
		value = value * 1024;
		break;

	case 'G':
		value = value * 1024 * 1024;
		break;

	case 'T':
		value = value * 1024 * 1024 * 1024;
		break;
	}

    return value;
}

unsigned long long CHelper::stringToSeconds(const std::string &intervalTxt)
{
    if (intervalTxt.empty())
    {
        return 0;
    }

    char unit = intervalTxt[intervalTxt.size() - 1];
    switch (unit)
    {
    case 'm':
        return std::stol(intervalTxt.substr(0, intervalTxt.size() - 1)) * 60000;

    case 's':
        return std::stol(intervalTxt.substr(0, intervalTxt.size() - 1)) * 1000;

    case 'h':
    default:
        return std::stol(intervalTxt.substr(0, intervalTxt.size() - 1)) * 3600000;
    }
}

long CHelper::getUID(const std::string& path)
{
	struct stat buf = { 0 };
    stat(path.c_str(), &buf);
    return buf.st_uid;
}

void CHelper::getHourMinute(const std::string & startTime, int &hour, int &minute)
{
    StringTokenizer st(startTime, ":");

    if (st.count() != 2 || !NumberParser::tryParse(st[0], hour) || !NumberParser::tryParse(st[1], minute))
    {
        throw Poco::Exception("Time " + startTime + " - invalid values");
    }

    if (minute < 0 || minute > 59)
    {
        throw Poco::Exception("Invalid minute: " + std::to_string(minute));
    }

    if (hour < 0 || hour > 23)
    {
        throw Poco::Exception("Invalid hour: " + std::to_string(hour));
    }
}

Poco::LocalDateTime CHelper::getNextHour()
{
	LocalDateTime now;
	LocalDateTime planned(now.year(), now.month(), now.day(), now.hour(), 0);
    Timespan hour(0, 1, 0, 0, 0);
    return planned + hour;
}

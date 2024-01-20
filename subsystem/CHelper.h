//
// Created by zielin on 01.11.23.
//

#ifndef STORAGE_CHELPER_H
#define STORAGE_CHELPER_H

#include <Poco/LocalDateTime.h>

#include <string>

class CHelper
{
public:
	static unsigned long long stringToKBytes(const std::string& str);
    static unsigned long long stringToSeconds(const std::string & str);
    static long getUID(const std::string& path);
    static void getHourMinute(const std::string &startTime, int & hour, int & minute);
    static Poco::LocalDateTime getNextHour();
};


#endif //STORAGE_CHELPER_H

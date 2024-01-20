#include "CByteCounter.h"

//#include <iomanip>
#include <sstream>
#include <Poco/NumberFormatter.h>

CByteCounter::CByteCounter()
= default;

void CByteCounter::reset()
{
    m_bytes = 0;
    m_KBytes = 0;
    m_MBytes = 0;
    m_GBytes = 0;
    m_TBytes = 0;
}

int CByteCounter::getBytes() const
{
    return m_bytes;
}

int CByteCounter::getKBytes() const
{
    return m_KBytes;
}

int CByteCounter::getMBytes() const
{
    return m_MBytes;
}

int CByteCounter::getGBytes() const
{
    return m_GBytes;
}

int CByteCounter::getTBytes() const
{
    return m_TBytes;
}

void CByteCounter::addBytes(unsigned long long bytes)
{
    m_bytes += bytes;

    if (m_bytes < 1024)
    {
        return;
    }

    unsigned long long kb = m_bytes / 1024;
    m_bytes -= kb * 1024;
    addKBytes(kb);
}

void CByteCounter::addKBytes(unsigned long long KBytes)
{
    m_KBytes += KBytes;

    if (m_KBytes < 1024)
    {
        return;
    }

    unsigned long long mb = m_KBytes / 1024;
    m_KBytes -= mb * 1024;
    addMBytes(mb);
}

void CByteCounter::addMBytes(unsigned long long MBytes)
{
    m_MBytes += MBytes;

    if (m_MBytes < 1024)
    {
        return;
    }

    unsigned long long gb = m_MBytes / 1024;
    m_MBytes -= gb * 1024;
    addGBytes(gb);
}

void CByteCounter::addGBytes(unsigned long long GBytes)
{
    m_GBytes += GBytes;

    if (m_GBytes < 1024)
    {
        return;
    }

    unsigned long long tb = m_GBytes / 1024;
    m_GBytes -= tb * 1024;
    addTBytes(tb);
}

void CByteCounter::addTBytes(unsigned long long TBytes)
{
    m_TBytes += TBytes;
}

std::string CByteCounter::toString(int precision) const
{
    std::stringstream ss;

    if (m_TBytes > 0)
    {
        ss << Poco::NumberFormatter::format((double) m_TBytes + ((m_GBytes + (m_MBytes / (double) 1024)) / (double) 1024), precision) << " TB";
    }
    else if (m_GBytes > 0)
    {
        ss << Poco::NumberFormatter::format((double) m_GBytes + (m_MBytes + (m_KBytes / (double) 1024)) / (double) 1024, precision) << " GB";
    }
    else if (m_MBytes > 0)
    {
        ss << Poco::NumberFormatter::format((double) m_MBytes + (m_KBytes + (m_KBytes / (double) 1024)) / (double) 1024, precision) << " MB";
    }
    else if (m_KBytes > 0)
    {
        ss << Poco::NumberFormatter::format((double) m_KBytes + (m_bytes / (double) 1024), precision) << " KB";
    }
    else if (m_bytes > 0)
    {
        ss << m_bytes << " B";
    }

    return ss.str();
}

unsigned long long CByteCounter::getTotalBytes() const
{
    return getBytes() + 1024 * getKBytes() + (1024 ^ 2) * getMBytes() + (1024 ^ 3) * getGBytes() + (1024 ^ 4) * getTBytes();
}

unsigned long long CByteCounter::getTotalKBytes() const
{
    return (getBytes() > 0 ? 1 : 0) + getKBytes() + 1024 * getMBytes() + (1024 ^ 2) * getGBytes() + (1024 ^ 3) * getTBytes();
}

unsigned long long CByteCounter::getTotalMBytes() const
{
    return (((getBytes() > 0 ? 1 : 0) + getKBytes()) > 0 ? 1 : 0) + getMBytes() + 1024 * getGBytes() + (1024 ^ 2) * getTBytes();
}

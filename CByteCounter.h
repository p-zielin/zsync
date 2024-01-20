#ifndef CBYTECOUNTER_H
#define CBYTECOUNTER_H

#include <string>

/*!
 * \brief Klasa zliczajaca bajty, KB, MB, GB, TB.
 */
class CByteCounter
{
public:
    CByteCounter();

    void reset();

    [[nodiscard]] int getBytes() const;
    [[nodiscard]] int getKBytes() const;
    [[nodiscard]] int getMBytes() const;
    [[nodiscard]] int getGBytes() const;
    [[nodiscard]] int getTBytes() const;

    void addBytes(unsigned long long bytes);
    void addKBytes(unsigned long long KBytes);
    void addMBytes(unsigned long long MBytes);
    void addGBytes(unsigned long long GBytes);
    void addTBytes(unsigned long long TBytes);

    [[nodiscard]] std::string toString(int precision = 2) const;

    [[nodiscard]] unsigned long long getTotalBytes() const;
    [[nodiscard]] unsigned long long getTotalKBytes() const;
    [[nodiscard]] unsigned long long getTotalMBytes() const;

protected:
    unsigned long long m_bytes = 0;
    unsigned long long m_KBytes = 0;
    unsigned long long m_MBytes = 0;
    unsigned long long m_GBytes = 0;
    unsigned long long m_TBytes = 0;
};

#endif // CBYTECOUNTER_H

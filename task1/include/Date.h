#pragma once
#include <ctime>
#include <cstdio>
#include <string>
#include <tuple>

struct Date
{
    int year = 0;
    int month = 0;
    int day = 0;

    bool operator<(const Date &o) const
    {
        return std::tie(year, month, day) < std::tie(o.year, o.month, o.day);
    }
    bool operator==(const Date &o) const
    {
        return std::tie(year, month, day) == std::tie(o.year, o.month, o.day);
    }
    std::string toString() const
    { // DD.MM.YYYY
        char buf[16];
        std::snprintf(buf, sizeof(buf), "%02d.%02d.%04d", day, month, year);
        return buf;
    }
};

inline bool isLeapYear(int y)
{
    return (y % 4 == 0 && y % 100 != 0) || y % 400 == 0;
}

inline int daysInMonth(int y, int m)
{
    static const int d[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    return (m == 2 && isLeapYear(y)) ? 29 : d[m - 1];
}

inline Date today()
{
    std::time_t t = std::time(nullptr);
    std::tm tm{};
#ifdef _WIN32
    localtime_s(&tm, &t);
#else
    localtime_r(&t, &tm);
#endif
    return Date{tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday};
}
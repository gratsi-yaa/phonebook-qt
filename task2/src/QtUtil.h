#pragma once
#include <QString>
#include <string>

inline std::string toStd(const QString &s) { return s.toUtf8().toStdString(); }

inline QString fromStd(const std::string &s)
{
    return QString::fromUtf8(s.data(), static_cast<qsizetype>(s.size()));
}
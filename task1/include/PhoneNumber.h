#pragma once
#include <string>

enum class PhoneType
{
    Work,
    Home,
    Service
}; // рабочий, домашний, служебный

struct PhoneNumber
{
    PhoneType type = PhoneType::Work;
    std::string number; // normalized: +7XXXXXXXXXX
};

inline const char *toString(PhoneType t)
{
    switch (t)
    {
    case PhoneType::Work:
        return "work";
    case PhoneType::Home:
        return "home";
    case PhoneType::Service:
        return "service";
    }
    return "work";
}
#pragma once
#include <string>
#include "Date.h"

// Each function trims the input, checks it with regular expressions,
// and on success writes the cleaned value to `out`.
class Validator
{
public:
    static std::string trim(const std::string &s);
    static bool normalizeName(const std::string &in, std::string &out);
    static bool normalizeAddress(const std::string &in, std::string &out);
    static bool normalizeEmail(const std::string &in, std::string &out);
    static bool normalizePhone(const std::string &in, std::string &out);
    static bool parseDate(const std::string &in, Date &out); // DD.MM.YYYY
};
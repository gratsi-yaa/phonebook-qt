#include "Validator.h"
#include <regex>
#include "Utf8.h"

std::string Validator::trim(const std::string &s)
{
    const char *ws = " \t\r\n";
    std::size_t b = s.find_first_not_of(ws);
    if (b == std::string::npos)
        return "";
    std::size_t e = s.find_last_not_of(ws);
    return s.substr(b, e - b + 1);
}

// Letters (Latin + Cyrillic) and digits, '-' and ' ' inside.
// Must start with a letter; must not end with '-' (or space after trim).
bool Validator::normalizeName(const std::string &in, std::string &out)
{
    std::string t = trim(in);
    std::wstring w;
    if (!utf8ToWide(t, w))
        return false;
    static const std::wregex re(
        L"^[A-Za-z\u0401\u0410-\u044F\u0451]"
        L"(?:[A-Za-z0-9\u0401\u0410-\u044F\u0451 -]*"
        L"[A-Za-z0-9\u0401\u0410-\u044F\u0451])?$");
    if (!std::regex_match(w, re))
        return false;
    out = t;
    return true;
}

// user@domain: Latin letters and digits; spaces around '@' are removed.
bool Validator::normalizeEmail(const std::string &in, std::string &out)
{
    std::string t = trim(in);
    std::size_t at = t.find('@');
    if (at == std::string::npos || t.find('@', at + 1) != std::string::npos)
        return false;
    std::string user = trim(t.substr(0, at));
    std::string domain = trim(t.substr(at + 1));
    static const std::regex ruser("^[A-Za-z0-9]+$");
    static const std::regex rdomain("^[A-Za-z0-9]+(\\.[A-Za-z0-9]+)*$");
    if (!std::regex_match(user, ruser) || !std::regex_match(domain, rdomain))
        return false;
    out = user + "@" + domain;
    return true;
}

// Accepts the 6 formats of the assignment, stores as +7XXXXXXXXXX.
bool Validator::normalizePhone(const std::string &in, std::string &out)
{
    std::string t = trim(in);
    static const std::regex r1(R"(^(\+7|8)\d{10}$)");
    static const std::regex r2(R"(^(\+7|8)\(\d{3}\)\d{7}$)");
    static const std::regex r3(R"(^(\+7|8)\(\d{3}\)\d{3}-\d{2}-\d{2}$)");
    if (!(std::regex_match(t, r1) || std::regex_match(t, r2) ||
          std::regex_match(t, r3)))
        return false;
    std::string digits;
    for (char c : t)
        if (c >= '0' && c <= '9')
            digits += c;
    out = "+7" + digits.substr(digits.size() - 10);
    return true;
}

// DD.MM.YYYY, real calendar date (leap years), strictly before today.
bool Validator::parseDate(const std::string &in, Date &out)
{
    std::string t = trim(in);
    static const std::regex r(R"(^(\d{1,2})\.(\d{1,2})\.(\d{4})$)");
    std::smatch m;
    if (!std::regex_match(t, m, r))
        return false;
    int d = std::stoi(m[1].str());
    int mo = std::stoi(m[2].str());
    int y = std::stoi(m[3].str());
    if (y < 1 || mo < 1 || mo > 12)
        return false;
    if (d < 1 || d > daysInMonth(y, mo))
        return false;
    Date date{y, mo, d};
    if (!(date < today()))
        return false;
    out = date;
    return true;
}
// Letters (Latin + Cyrillic), digits, spaces and . , / # ( ) № -
bool Validator::normalizeAddress(const std::string &in, std::string &out)
{
    std::string t = trim(in);
    std::wstring w;
    if (t.empty() || !utf8ToWide(t, w))
        return false;
    static const std::wregex re(
        L"^[A-Za-z0-9\u0401\u0410-\u044F\u0451 .,/#()\u2116-]+$");
    if (!std::regex_match(w, re))
        return false;
    out = t;
    return true;
}
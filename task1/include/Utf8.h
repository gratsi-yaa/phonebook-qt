#pragma once
#include <string>

// Decodes a UTF-8 string into std::wstring. Returns false on invalid input.
inline bool utf8ToWide(const std::string &s, std::wstring &out)
{
    out.clear();
    for (std::size_t i = 0; i < s.size();)
    {
        unsigned char c = static_cast<unsigned char>(s[i]);
        char32_t cp = 0;
        int len = 0;
        if (c < 0x80)
        {
            cp = c;
            len = 1;
        }
        else if ((c >> 5) == 0x6)
        {
            cp = c & 0x1F;
            len = 2;
        }
        else if ((c >> 4) == 0xE)
        {
            cp = c & 0x0F;
            len = 3;
        }
        else if ((c >> 3) == 0x1E)
        {
            cp = c & 0x07;
            len = 4;
        }
        else
            return false;
        if (i + len > s.size())
            return false;
        for (int k = 1; k < len; ++k)
        {
            unsigned char cc = static_cast<unsigned char>(s[i + k]);
            if ((cc >> 6) != 0x2)
                return false;
            cp = (cp << 6) | (cc & 0x3F);
        }
        i += len;
        if (sizeof(wchar_t) == 2 && cp > 0xFFFF)
        { // surrogate pair (Windows)
            cp -= 0x10000;
            out.push_back(static_cast<wchar_t>(0xD800 + (cp >> 10)));
            out.push_back(static_cast<wchar_t>(0xDC00 + (cp & 0x3FF)));
        }
        else
        {
            out.push_back(static_cast<wchar_t>(cp));
        }
    }
    return true;
}
#include "FileStorage.h"
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <stdexcept>
#include "Validator.h"

namespace
{

    std::string escape(const std::string &s)
    {
        std::string r;
        for (char c : s)
        {
            switch (c)
            {
            case '\\':
                r += "\\\\";
                break;
            case '\t':
                r += "\\t";
                break;
            case '\n':
                r += "\\n";
                break;
            case '\r':
                r += "\\r";
                break;
            default:
                r += c;
            }
        }
        return r;
    }

    std::string unescape(const std::string &s)
    {
        std::string r;
        for (std::size_t i = 0; i < s.size(); ++i)
        {
            if (s[i] == '\\' && i + 1 < s.size())
            {
                char n = s[++i];
                r += (n == 't') ? '\t' : (n == 'n') ? '\n'
                                     : (n == 'r')   ? '\r'
                                                    : n;
            }
            else
            {
                r += s[i];
            }
        }
        return r;
    }

    std::vector<std::string> split(const std::string &s, char sep)
    {
        std::vector<std::string> parts;
        std::string cur;
        for (char c : s)
        {
            if (c == sep)
            {
                parts.push_back(cur);
                cur.clear();
            }
            else
                cur += c;
        }
        parts.push_back(cur);
        return parts;
    }

    std::string formatLine(const Contact &c)
    {
        std::string phones;
        for (const PhoneNumber &p : c.phones)
        {
            if (!phones.empty())
                phones += ';';
            phones += std::string(toString(p.type)) + ":" + p.number;
        }
        return std::to_string(c.id) + '\t' + escape(c.firstName) + '\t' +
               escape(c.lastName) + '\t' + escape(c.middleName) + '\t' +
               escape(c.address) + '\t' + escape(c.email) + '\t' +
               (c.birthDate ? c.birthDate->toString() : "") + '\t' + phones;
    }

    Contact parseLine(const std::string &line)
    {
        std::vector<std::string> f = split(line, '\t');
        if (f.size() != 8)
            throw std::runtime_error("wrong number of fields");
        Contact c;
        c.id = std::stoi(f[0]);
        c.firstName = unescape(f[1]);
        c.lastName = unescape(f[2]);
        c.middleName = unescape(f[3]);
        c.address = unescape(f[4]);
        c.email = unescape(f[5]);
        if (!f[6].empty())
        {
            Date d;
            if (!Validator::parseDate(f[6], d))
                throw std::runtime_error("bad birth date");
            c.birthDate = d;
        }
        for (const std::string &item : split(f[7], ';'))
        {
            if (item.empty())
                continue;
            std::size_t colon = item.find(':');
            PhoneNumber p;
            if (colon == std::string::npos ||
                !phoneTypeFromString(item.substr(0, colon), p.type))
                throw std::runtime_error("bad phone entry");
            p.number = item.substr(colon + 1);
            c.phones.push_back(p);
        }
        return c;
    }

} // namespace

FileStorage::FileStorage(std::string path) : m_path(std::move(path)) { load(); }

void FileStorage::load()
{
    std::ifstream in(m_path, std::ios::binary);
    if (!in)
        return; // no file yet: the phone book starts empty
    std::string line;
    int lineNo = 0;
    while (std::getline(in, line))
    {
        ++lineNo;
        if (!line.empty() && line.back() == '\r')
            line.pop_back();
        if (line.empty())
            continue;
        try
        {
            m_contacts.push_back(parseLine(line));
        }
        catch (const std::exception &e)
        {
            throw std::runtime_error(m_path + ", line " + std::to_string(lineNo) +
                                     ": " + e.what());
        }
        m_nextId = std::max(m_nextId, m_contacts.back().id + 1);
    }
}

void FileStorage::save() const
{
    std::filesystem::path parent = std::filesystem::path(m_path).parent_path();
    if (!parent.empty())
        std::filesystem::create_directories(parent);
    std::ofstream out(m_path, std::ios::binary | std::ios::trunc);
    if (!out)
        throw std::runtime_error("Cannot write file: " + m_path);
    for (const Contact &c : m_contacts)
        out << formatLine(c) << '\n';
    if (!out)
        throw std::runtime_error("Error while writing file: " + m_path);
}

int FileStorage::add(const Contact &c)
{
    Contact copy = c;
    copy.id = m_nextId++;
    m_contacts.push_back(copy);
    save();
    return copy.id;
}

void FileStorage::update(const Contact &c)
{
    auto it = std::find_if(m_contacts.begin(), m_contacts.end(),
                           [&](const Contact &x)
                           { return x.id == c.id; });
    if (it == m_contacts.end())
        throw std::runtime_error("Contact not found");
    *it = c;
    save();
}

void FileStorage::remove(int id)
{
    auto it = std::remove_if(m_contacts.begin(), m_contacts.end(),
                             [&](const Contact &x)
                             { return x.id == id; });
    if (it == m_contacts.end())
        throw std::runtime_error("Contact not found");
    m_contacts.erase(it, m_contacts.end());
    save();
}
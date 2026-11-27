#include "PhoneBook.h"
#include <algorithm>
#include <cstdio>
#include <stdexcept>
#include "Utf8.h"
#include "Validator.h"

PhoneBook::PhoneBook(std::unique_ptr<IContactStorage> storage)
    : m_storage(std::move(storage)), m_contacts(m_storage->loadAll()) {}

const Contact *PhoneBook::find(int id) const
{
    for (const Contact &c : m_contacts)
        if (c.id == id)
            return &c;
    return nullptr;
}

void PhoneBook::validate(Contact &c)
{
    std::string v;
    if (!Validator::normalizeName(c.lastName, v))
        throw std::invalid_argument("Invalid last name");
    c.lastName = v;
    if (!Validator::normalizeName(c.firstName, v))
        throw std::invalid_argument("Invalid first name");
    c.firstName = v;

    c.middleName = Validator::trim(c.middleName);
    if (!c.middleName.empty())
    {
        if (!Validator::normalizeName(c.middleName, v))
            throw std::invalid_argument("Invalid middle name");
        c.middleName = v;
    }

    c.address = Validator::trim(c.address);
    if (!c.address.empty())
    {
        if (!Validator::normalizeAddress(c.address, v))
            throw std::invalid_argument("Invalid address");
        c.address = v;
    }

    if (!Validator::normalizeEmail(c.email, v))
        throw std::invalid_argument("Invalid e-mail");
    c.email = v;

    if (c.birthDate)
    {
        const Date &d = *c.birthDate;
        if (d.year < 1 || d.month < 1 || d.month > 12 || d.day < 1 ||
            d.day > daysInMonth(d.year, d.month) || !(d < today()))
            throw std::invalid_argument("Invalid birth date");
    }

    if (c.phones.empty())
        throw std::invalid_argument("At least one phone number is required");
    for (PhoneNumber &p : c.phones)
    {
        if (!Validator::normalizePhone(p.number, v))
            throw std::invalid_argument("Invalid phone number: " + p.number);
        p.number = v;
    }
}

int PhoneBook::add(Contact c)
{
    validate(c);
    c.id = m_storage->add(c);
    m_contacts.push_back(c);
    return c.id;
}

void PhoneBook::update(Contact c)
{
    validate(c);
    auto it = std::find_if(m_contacts.begin(), m_contacts.end(),
                           [&](const Contact &x)
                           { return x.id == c.id; });
    if (it == m_contacts.end())
        throw std::invalid_argument("No such contact");
    m_storage->update(c);
    *it = c;
}

void PhoneBook::remove(int id)
{
    if (!find(id))
        throw std::invalid_argument("No such contact");
    m_storage->remove(id);
    m_contacts.erase(std::remove_if(m_contacts.begin(), m_contacts.end(),
                                    [&](const Contact &x)
                                    { return x.id == id; }),
                     m_contacts.end());
}

static std::wstring sortKey(const Contact &c, SortField f)
{
    switch (f)
    {
    case SortField::LastName:
        return lowerKey(c.lastName);
    case SortField::FirstName:
        return lowerKey(c.firstName);
    case SortField::MiddleName:
        return lowerKey(c.middleName);
    case SortField::Address:
        return lowerKey(c.address);
    case SortField::Email:
        return lowerKey(c.email);
    case SortField::Phone:
        return c.phones.empty() ? L"" : lowerKey(c.phones[0].number);
    case SortField::BirthDate:
    {
        if (!c.birthDate)
            return L"";
        char buf[16];
        std::snprintf(buf, sizeof(buf), "%04d%02d%02d", c.birthDate->year,
                      c.birthDate->month, c.birthDate->day);
        return lowerKey(buf);
    }
    }
    return L"";
}

std::vector<Contact> PhoneBook::sorted(SortField field, bool ascending) const
{
    std::vector<Contact> result = m_contacts;
    std::stable_sort(result.begin(), result.end(),
                     [&](const Contact &a, const Contact &b)
                     {
                         return ascending ? sortKey(a, field) < sortKey(b, field)
                                          : sortKey(b, field) < sortKey(a, field);
                     });
    return result;
}

std::vector<Contact> PhoneBook::search(const ContactFilter &f) const
{
    auto ok = [](const std::string &value, const std::string &query)
    {
        std::string q = Validator::trim(query);
        return q.empty() || containsIgnoreCase(value, q);
    };
    std::vector<Contact> result;
    for (const Contact &c : m_contacts)
    {
        bool phoneOk = Validator::trim(f.phone).empty();
        for (const PhoneNumber &p : c.phones)
            if (ok(p.number, f.phone))
                phoneOk = true;
        if (ok(c.lastName, f.lastName) && ok(c.firstName, f.firstName) &&
            ok(c.middleName, f.middleName) && ok(c.address, f.address) &&
            ok(c.email, f.email) && phoneOk &&
            ok(c.birthDate ? c.birthDate->toString() : "", f.birthDate))
            result.push_back(c);
    }
    return result;
}
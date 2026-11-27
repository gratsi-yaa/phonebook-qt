#pragma once
#include <memory>
#include <string>
#include <vector>
#include "Contact.h"
#include "IContactStorage.h"

enum class SortField
{
    LastName,
    FirstName,
    MiddleName,
    Address,
    BirthDate,
    Email,
    Phone
};

// Search by several fields at once: every non-empty field must match
// (case-insensitive substring).
struct ContactFilter
{
    std::string lastName, firstName, middleName, address, email, phone, birthDate;
};

class PhoneBook
{
public:
    explicit PhoneBook(std::unique_ptr<IContactStorage> storage);

    const std::vector<Contact> &contacts() const { return m_contacts; }
    const Contact *find(int id) const;

    int add(Contact c);     // throws std::invalid_argument if invalid
    void update(Contact c); // throws std::invalid_argument if invalid
    void remove(int id);

    std::vector<Contact> sorted(SortField field, bool ascending = true) const;
    std::vector<Contact> search(const ContactFilter &filter) const;

    // Checks all rules and normalizes fields in place.
    static void validate(Contact &c);

private:
    std::unique_ptr<IContactStorage> m_storage;
    std::vector<Contact> m_contacts;
};
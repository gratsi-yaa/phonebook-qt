#include "ConsoleUI.h"
#include <iostream>
#include <optional>
#include "Validator.h"

namespace
{

    struct InputClosed
    {
    }; // thrown when stdin ends (Ctrl+D / Ctrl+Z)

    using Normalizer = bool (*)(const std::string &, std::string &);

    const char *kNameHint = "Letters, digits, '-' and spaces; must start with a letter, must not end with '-'.";
    const char *kAddressHint = "Letters, digits, spaces and . , / # ( ) - only.";
    const char *kEmailHint = "Expected user@domain (Latin letters and digits).";
    const char *kPhoneHint = "Formats: +78121234567, 88121234567, +7(812)1234567, 8(812)1234567, +7(812)123-45-67, 8(812)123-45-67.";
    const char *kDateHint = "Use DD.MM.YYYY: a real date, strictly before today.";

    std::string readLine(const std::string &prompt)
    {
        std::cout << prompt << std::flush;
        std::string s;
        if (!std::getline(std::cin, s))
            throw InputClosed{};
        return Validator::trim(s);
    }

    std::string askRequired(const std::string &prompt, Normalizer norm, const char *hint)
    {
        for (;;)
        {
            std::string out;
            if (norm(readLine(prompt), out))
                return out;
            std::cout << "  Invalid value. " << hint << "\n";
        }
    }

    std::string askOptional(const std::string &prompt, Normalizer norm, const char *hint)
    {
        for (;;)
        {
            std::string raw = readLine(prompt);
            std::string out;
            if (raw.empty())
                return "";
            if (norm(raw, out))
                return out;
            std::cout << "  Invalid value. " << hint << "\n";
        }
    }

    std::string editRequired(const std::string &label, const std::string &current,
                             Normalizer norm, const char *hint)
    {
        for (;;)
        {
            std::string raw = readLine(label + " [" + current + "] (Enter = keep): ");
            std::string out;
            if (raw.empty())
                return current;
            if (norm(raw, out))
                return out;
            std::cout << "  Invalid value. " << hint << "\n";
        }
    }

    std::string editOptional(const std::string &label, const std::string &current,
                             Normalizer norm, const char *hint)
    {
        for (;;)
        {
            std::string raw = readLine(label + " [" + current + "] (Enter = keep, '-' = clear): ");
            std::string out;
            if (raw.empty())
                return current;
            if (raw == "-")
                return "";
            if (norm(raw, out))
                return out;
            std::cout << "  Invalid value. " << hint << "\n";
        }
    }

    std::optional<Date> askDate(const std::string &prompt)
    {
        for (;;)
        {
            std::string raw = readLine(prompt);
            if (raw.empty())
                return std::nullopt;
            Date d;
            if (Validator::parseDate(raw, d))
                return d;
            std::cout << "  Invalid date. " << kDateHint << "\n";
        }
    }

    std::optional<Date> editDate(const std::optional<Date> &current)
    {
        std::string label = std::string("Birth date [") +
                            (current ? current->toString() : "none") +
                            "] (Enter = keep, '-' = clear): ";
        for (;;)
        {
            std::string raw = readLine(label);
            if (raw.empty())
                return current;
            if (raw == "-")
                return std::nullopt;
            Date d;
            if (Validator::parseDate(raw, d))
                return d;
            std::cout << "  Invalid date. " << kDateHint << "\n";
        }
    }

    PhoneNumber askPhone()
    {
        PhoneNumber p;
        for (;;)
        {
            std::string t = readLine("  Type (1 = work, 2 = home, 3 = service): ");
            if (t == "1")
                p.type = PhoneType::Work;
            else if (t == "2")
                p.type = PhoneType::Home;
            else if (t == "3")
                p.type = PhoneType::Service;
            else
            {
                std::cout << "  Enter 1, 2 or 3.\n";
                continue;
            }
            break;
        }
        p.number = askRequired("  Number: ", &Validator::normalizePhone, kPhoneHint);
        return p;
    }

    void editPhones(std::vector<PhoneNumber> &phones)
    {
        for (;;)
        {
            std::cout << "Phone numbers:\n";
            for (std::size_t i = 0; i < phones.size(); ++i)
                std::cout << "  " << (i + 1) << ") " << toString(phones[i].type)
                          << ": " << phones[i].number << "\n";
            std::string a = readLine("(a) add, (d) delete, Enter = done: ");
            if (a.empty())
                return;
            if (a == "a")
            {
                phones.push_back(askPhone());
            }
            else if (a == "d")
            {
                if (phones.size() == 1)
                {
                    std::cout << "  A contact needs at least one phone number.\n";
                    continue;
                }
                std::string n = readLine("  Number of the phone to delete: ");
                int idx = -1;
                try
                {
                    idx = std::stoi(n);
                }
                catch (...)
                {
                }
                if (idx >= 1 && idx <= static_cast<int>(phones.size()))
                    phones.erase(phones.begin() + (idx - 1));
                else
                    std::cout << "  Invalid index.\n";
            }
        }
    }

    void printContact(const Contact &c)
    {
        std::cout << "[" << c.id << "] " << c.lastName << " " << c.firstName;
        if (!c.middleName.empty())
            std::cout << " " << c.middleName;
        std::cout << "\n    email:   " << c.email;
        if (c.birthDate)
            std::cout << "\n    born:    " << c.birthDate->toString();
        if (!c.address.empty())
            std::cout << "\n    address: " << c.address;
        for (const PhoneNumber &p : c.phones)
            std::cout << "\n    phone (" << toString(p.type) << "): " << p.number;
        std::cout << "\n";
    }

    void printList(const std::vector<Contact> &list)
    {
        if (list.empty())
        {
            std::cout << "No contacts.\n";
            return;
        }
        for (const Contact &c : list)
        {
            printContact(c);
            std::cout << "\n";
        }
        std::cout << list.size() << " contact(s).\n";
    }

    int askId(const std::string &prompt)
    {
        try
        {
            return std::stoi(readLine(prompt));
        }
        catch (...)
        {
            return -1;
        }
    }

} // namespace

void ConsoleUI::run()
{
    try
    {
        for (;;)
        {
            std::cout << "\n=== Phone book ===\n"
                         "1. List contacts\n2. Add contact\n3. Edit contact\n"
                         "4. Delete contact\n5. Sort\n6. Search\n0. Exit\n";
            std::string choice = readLine("> ");
            try
            {
                if (choice == "1")
                    listAll();
                else if (choice == "2")
                    addContact();
                else if (choice == "3")
                    editContact();
                else if (choice == "4")
                    removeContact();
                else if (choice == "5")
                    sortContacts();
                else if (choice == "6")
                    searchContacts();
                else if (choice == "0")
                    return;
                else
                    std::cout << "Unknown choice.\n";
            }
            catch (const std::exception &e)
            {
                std::cout << "Error: " << e.what() << "\n";
            }
        }
    }
    catch (const InputClosed &)
    {
        std::cout << "\n";
    }
}

void ConsoleUI::listAll() { printList(m_book.contacts()); }

void ConsoleUI::addContact()
{
    Contact c;
    c.lastName = askRequired("Last name (required): ", &Validator::normalizeName, kNameHint);
    c.firstName = askRequired("First name (required): ", &Validator::normalizeName, kNameHint);
    c.middleName = askOptional("Middle name (Enter to skip): ", &Validator::normalizeName, kNameHint);
    c.address = askOptional("Address (Enter to skip): ", &Validator::normalizeAddress, kAddressHint);
    c.email = askRequired("E-mail (required): ", &Validator::normalizeEmail, kEmailHint);
    c.birthDate = askDate("Birth date DD.MM.YYYY (Enter to skip): ");
    std::cout << "Phone number 1 (required):\n";
    c.phones.push_back(askPhone());
    while (readLine("Add another phone number? (y/N): ") == "y")
    {
        std::cout << "Phone number " << (c.phones.size() + 1) << ":\n";
        c.phones.push_back(askPhone());
    }
    int id = m_book.add(c);
    std::cout << "Contact added with id " << id << ".\n";
}

void ConsoleUI::editContact()
{
    const Contact *found = m_book.find(askId("Id of the contact to edit: "));
    if (!found)
    {
        std::cout << "No contact with this id.\n";
        return;
    }
    Contact c = *found;
    c.lastName = editRequired("Last name", c.lastName, &Validator::normalizeName, kNameHint);
    c.firstName = editRequired("First name", c.firstName, &Validator::normalizeName, kNameHint);
    c.middleName = editOptional("Middle name", c.middleName, &Validator::normalizeName, kNameHint);
    c.address = editOptional("Address", c.address, &Validator::normalizeAddress, kAddressHint);
    c.email = editRequired("E-mail", c.email, &Validator::normalizeEmail, kEmailHint);
    c.birthDate = editDate(c.birthDate);
    editPhones(c.phones);
    m_book.update(c);
    std::cout << "Contact updated.\n";
}

void ConsoleUI::removeContact()
{
    int id = askId("Id of the contact to delete: ");
    const Contact *found = m_book.find(id);
    if (!found)
    {
        std::cout << "No contact with this id.\n";
        return;
    }
    printContact(*found);
    if (readLine("Delete this contact? (y/N): ") == "y")
    {
        m_book.remove(id);
        std::cout << "Contact deleted.\n";
    }
}

void ConsoleUI::sortContacts()
{
    std::cout << "Sort by: 1 last name, 2 first name, 3 middle name, 4 address,\n"
                 "         5 birth date, 6 e-mail, 7 phone\n";
    std::string s = readLine("> ");
    SortField field;
    if (s == "1")
        field = SortField::LastName;
    else if (s == "2")
        field = SortField::FirstName;
    else if (s == "3")
        field = SortField::MiddleName;
    else if (s == "4")
        field = SortField::Address;
    else if (s == "5")
        field = SortField::BirthDate;
    else if (s == "6")
        field = SortField::Email;
    else if (s == "7")
        field = SortField::Phone;
    else
    {
        std::cout << "Unknown choice.\n";
        return;
    }
    bool asc = readLine("Order: 1 ascending, 2 descending: ") != "2";
    printList(m_book.sorted(field, asc));
}

void ConsoleUI::searchContacts()
{
    std::cout << "Fill in one or several fields (Enter = ignore the field).\n";
    ContactFilter f;
    f.lastName = readLine("Last name contains: ");
    f.firstName = readLine("First name contains: ");
    f.middleName = readLine("Middle name contains: ");
    f.address = readLine("Address contains: ");
    f.email = readLine("E-mail contains: ");
    f.phone = readLine("Phone contains: ");
    f.birthDate = readLine("Birth date contains (e.g. 1999 or 05.1999): ");
    printList(m_book.search(f));
}
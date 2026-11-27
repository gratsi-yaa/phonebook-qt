#include <cstdio>
#include <iostream>
#include <memory>
#include "FileStorage.h"
#include "PhoneBook.h"

static int failures = 0;
#define CHECK(cond)                                                   \
    do                                                                \
    {                                                                 \
        if (!(cond))                                                  \
        {                                                             \
            ++failures;                                               \
            std::cerr << "FAIL line " << __LINE__ << ": " #cond "\n"; \
        }                                                             \
    } while (0)

static Contact make(const std::string &last, const std::string &first,
                    const std::string &email, const std::string &phone)
{
    Contact c;
    c.lastName = last;
    c.firstName = first;
    c.email = email;
    c.phones.push_back({PhoneType::Work, phone});
    return c;
}

template <typename F>
static bool throwsInvalid(F f)
{
    try
    {
        f();
    }
    catch (const std::invalid_argument &)
    {
        return true;
    }
    return false;
}

int main()
{
    const std::string path = "test_contacts.txt";
    std::remove(path.c_str());
    {
        PhoneBook book(std::make_unique<FileStorage>(path));
        book.add(make("Петров", "Иван", "ivan@mail.com", "8(812)123-45-67"));
        Contact c2 = make("Алексеев", "Анна", "anna@mail.com", "+78125550000");
        c2.address = "Moscow, Lenina 1";
        c2.birthDate = Date{1999, 5, 17};
        book.add(c2);

        CHECK(book.contacts().size() == 2);
        CHECK(book.contacts()[0].phones[0].number == "+78121234567");

        auto s = book.sorted(SortField::LastName);
        CHECK(s[0].lastName == "Алексеев");
        auto d = book.sorted(SortField::LastName, false);
        CHECK(d[0].lastName == "Петров");

        ContactFilter f;
        f.email = "ANNA";
        CHECK(book.search(f).size() == 1);
        f = ContactFilter{};
        f.lastName = "петр";
        f.firstName = "ива";
        CHECK(book.search(f).size() == 1);
        f.firstName = "анна";
        CHECK(book.search(f).empty());

        CHECK(throwsInvalid([&]
                            { book.add(make("-Bad", "X", "x@y.com", "88121234567")); }));
        CHECK(throwsInvalid([&]
                            { book.add(make("Ivanov", "Ivan", "bad", "88121234567")); }));
        CHECK(throwsInvalid([&]
                            { book.add(make("Ivanov", "Ivan", "a@b.com", "123")); }));
        Contact noPhone = make("Ivanov", "Ivan", "a@b.com", "88121234567");
        noPhone.phones.clear();
        CHECK(throwsInvalid([&]
                            { book.add(noPhone); }));
        CHECK(book.contacts().size() == 2);
    }
    { // data survives a restart
        PhoneBook book(std::make_unique<FileStorage>(path));
        CHECK(book.contacts().size() == 2);
        CHECK(book.contacts()[1].birthDate &&
              book.contacts()[1].birthDate->toString() == "17.05.1999");
        Contact edited = book.contacts()[0];
        edited.firstName = "Пётр";
        book.update(edited);
        book.remove(book.contacts()[1].id);
        CHECK(book.contacts().size() == 1);
    }
    {
        PhoneBook book(std::make_unique<FileStorage>(path));
        CHECK(book.contacts().size() == 1);
        CHECK(book.contacts()[0].firstName == "Пётр");
    }
    std::remove(path.c_str());

    if (failures == 0)
        std::cout << "All phonebook tests passed\n";
    return failures == 0 ? 0 : 1;
}
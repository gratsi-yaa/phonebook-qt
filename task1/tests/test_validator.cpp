#include <iostream>
#include "Validator.h"

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

int main()
{
    std::string o;
    // names
    CHECK(Validator::normalizeName("  Иван  ", o) && o == "Иван");
    CHECK(Validator::normalizeName("Анна-Мария", o));
    CHECK(Validator::normalizeName("Smith 2", o));
    CHECK(!Validator::normalizeName("-Иван", o));
    CHECK(!Validator::normalizeName("Иван-", o));
    CHECK(!Validator::normalizeName("1Иван", o));
    CHECK(!Validator::normalizeName("Ив@н", o));
    CHECK(!Validator::normalizeName("   ", o));
    // phones
    const char *okPhones[] = {"+78121234567", "88121234567", "+7(812)1234567",
                              "8(812)1234567", "+7(812)123-45-67",
                              "8(812)123-45-67"};
    for (const char *p : okPhones)
        CHECK(Validator::normalizePhone(p, o) && o == "+78121234567");
    CHECK(!Validator::normalizePhone("78121234567", o));
    CHECK(!Validator::normalizePhone("+7(812)123-4567", o));
    CHECK(!Validator::normalizePhone("+7812123456", o));
    // email
    CHECK(Validator::normalizeEmail("  user1 @ mail.com ", o) && o == "user1@mail.com");
    CHECK(!Validator::normalizeEmail("user@", o));
    CHECK(!Validator::normalizeEmail("@mail.com", o));
    CHECK(!Validator::normalizeEmail("us er@mail.com", o));
    CHECK(!Validator::normalizeEmail("юзер@mail.com", o));
    // dates
    Date d;
    CHECK(Validator::parseDate("29.02.2000", d));
    CHECK(!Validator::parseDate("29.02.1900", d));
    CHECK(Validator::parseDate("29.02.2004", d));
    CHECK(!Validator::parseDate("31.04.2000", d));
    CHECK(!Validator::parseDate("15.13.2000", d));
    CHECK(!Validator::parseDate("00.01.2000", d));
    CHECK(!Validator::parseDate("01.01.2999", d));
    CHECK(!Validator::parseDate("2000-01-01", d));

    if (failures == 0)
        std::cout << "All tests passed\n";
    return failures == 0 ? 0 : 1;
}
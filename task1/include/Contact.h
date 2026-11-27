#pragma once
#include <optional>
#include <string>
#include <vector>
#include "Date.h"
#include "PhoneNumber.h"

struct Contact
{
    int id = 0;                      // assigned by the storage
    std::string firstName;           // required
    std::string lastName;            // required
    std::string middleName;          // optional
    std::string address;             // optional
    std::string email;               // required
    std::optional<Date> birthDate;   // optional
    std::vector<PhoneNumber> phones; // at least 1 required, no upper limit
};
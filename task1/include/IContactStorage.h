#pragma once
#include <vector>
#include "Contact.h"

// Abstract storage: the phone book does not care where data lives
// (text file now, PostgreSQL in task 2). All methods throw
// std::runtime_error on failure.
class IContactStorage
{
public:
    virtual ~IContactStorage() = default;
    virtual std::vector<Contact> loadAll() = 0;
    virtual int add(const Contact &c) = 0; // returns the new id
    virtual void update(const Contact &c) = 0;
    virtual void remove(int id) = 0;
};
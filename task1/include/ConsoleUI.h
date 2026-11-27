#pragma once
#include "PhoneBook.h"

class ConsoleUI
{
public:
    explicit ConsoleUI(PhoneBook &book) : m_book(book) {}
    void run();

private:
    void listAll();
    void addContact();
    void editContact();
    void removeContact();
    void sortContacts();
    void searchContacts();

    PhoneBook &m_book;
};
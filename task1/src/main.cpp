#include <iostream>
#include <memory>
#include "ConsoleUI.h"
#include "FileStorage.h"
#include "PhoneBook.h"

#ifdef _WIN32
#include <windows.h>
#endif

int main(int argc, char *argv[])
{
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8); // Cyrillic in the console
    SetConsoleCP(CP_UTF8);
#endif
    // Optional argument: path to the data file.
    std::string path = (argc > 1) ? argv[1] : "data/contacts.txt";
    try
    {
        PhoneBook book(std::make_unique<FileStorage>(path));
        ConsoleUI ui(book);
        ui.run();
    }
    catch (const std::exception &e)
    {
        std::cerr << "Fatal error: " << e.what() << "\n";
        return 1;
    }
    return 0;
}
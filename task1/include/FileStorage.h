#pragma once
#include <string>
#include <vector>
#include "IContactStorage.h"

// Stores contacts in a UTF-8 text file, one contact per line,
// fields separated by TAB (special characters are escaped).
class FileStorage : public IContactStorage
{
public:
    explicit FileStorage(std::string path);

    std::vector<Contact> loadAll() override { return m_contacts; }
    int add(const Contact &c) override;
    void update(const Contact &c) override;
    void remove(int id) override;

private:
    void load();
    void save() const;

    std::string m_path;
    std::vector<Contact> m_contacts;
    int m_nextId = 1;
};
#pragma once
#include <QSqlDatabase>
#include <QString>
#include "IContactStorage.h"

// Stores contacts in a PostgreSQL database (Qt SQL, driver QPSQL).
// Two tables: contacts (1) --- (N) phones. All methods throw
// std::runtime_error on failure.
class PostgresStorage : public IContactStorage
{
public:
    struct Settings
    {
        QString host = "localhost";
        int port = 5432;
        QString database = "phonebook";
        QString user = "phonebook";
        QString password;
    };

    explicit PostgresStorage(const Settings &settings);
    ~PostgresStorage() override;

    std::vector<Contact> loadAll() override;
    int add(const Contact &c) override;
    void update(const Contact &c) override;
    void remove(int id) override;

private:
    void ensureSchema();
    void insertPhones(int contactId, const std::vector<PhoneNumber> &phones);

    QSqlDatabase m_db;
    QString m_connectionName;
};
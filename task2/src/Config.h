#pragma once
#include <QString>
#include <memory>
#include "IContactStorage.h"
#include "PostgresStorage.h"

// Application settings read from config.ini.
struct AppConfig
{
    enum class StorageType
    {
        File,
        Postgres
    };

    StorageType storage = StorageType::File;
    QString filePath = "data/contacts.txt";
    PostgresStorage::Settings postgres;

    // Missing file = defaults (file storage). Throws std::runtime_error
    // if a value is invalid.
    static AppConfig load(const QString &iniPath);
};

// Creates the storage chosen in the configuration.
std::unique_ptr<IContactStorage> createStorage(const AppConfig &config);
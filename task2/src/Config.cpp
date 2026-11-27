#include "Config.h"
#include <QFile>
#include <QSettings>
#include <stdexcept>
#include "FileStorage.h"
#include "QtUtil.h"

AppConfig AppConfig::load(const QString &iniPath)
{
    AppConfig cfg;
    if (!QFile::exists(iniPath))
        return cfg;

    QSettings s(iniPath, QSettings::IniFormat);
    const QString type = s.value("storage/type", "file").toString().trimmed().toLower();
    if (type == "file")
        cfg.storage = StorageType::File;
    else if (type == "postgres")
        cfg.storage = StorageType::Postgres;
    else
        throw std::runtime_error("config.ini: [storage] type must be 'file' or 'postgres'");

    cfg.filePath = s.value("file/path", cfg.filePath).toString();
    cfg.postgres.host = s.value("postgres/host", cfg.postgres.host).toString();
    cfg.postgres.port = s.value("postgres/port", cfg.postgres.port).toInt();
    cfg.postgres.database = s.value("postgres/database", cfg.postgres.database).toString();
    cfg.postgres.user = s.value("postgres/user", cfg.postgres.user).toString();
    cfg.postgres.password = s.value("postgres/password", cfg.postgres.password).toString();
    return cfg;
}

std::unique_ptr<IContactStorage> createStorage(const AppConfig &config)
{
    if (config.storage == AppConfig::StorageType::Postgres)
        return std::make_unique<PostgresStorage>(config.postgres);
    return std::make_unique<FileStorage>(toStd(config.filePath));
}
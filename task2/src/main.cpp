#include <QApplication>
#include <QDir>
#include <QMessageBox>
#include "Config.h"
#include "MainWindow.h"
#include "PhoneBook.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // Relative paths (config, data file) are relative to the executable.
    // Changing the working directory also avoids passing non-ASCII Windows
    // paths to std::ofstream.
    const QString appDir = QCoreApplication::applicationDirPath();
    QDir::setCurrent(appDir);

    try
    {
        // Optional argument: path to another configuration file.
        const QStringList args = app.arguments();
        const QString configPath = args.size() > 1 ? args[1] : QDir(appDir).absoluteFilePath("config.ini");

        const AppConfig config = AppConfig::load(configPath);
        PhoneBook book(createStorage(config));

        MainWindow window(book);
        window.setWindowTitle(window.windowTitle() + (config.storage == AppConfig::StorageType::Postgres
                                                          ? " [PostgreSQL]"
                                                          : " [file]"));
        window.show();
        return app.exec();
    }
    catch (const std::exception &e)
    {
        QMessageBox::critical(nullptr, "Phone book", QString::fromUtf8(e.what()));
        return 1;
    }
}
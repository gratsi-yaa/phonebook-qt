#include <QApplication>
#include <QDir>
#include <QMessageBox>
#include <memory>
#include "FileStorage.h"
#include "MainWindow.h"
#include "PhoneBook.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // The data file lives next to the executable. Changing the working
    // directory avoids passing non-ASCII Windows paths to std::ofstream.
    QDir::setCurrent(QCoreApplication::applicationDirPath());

    try
    {
        PhoneBook book(std::make_unique<FileStorage>("data/contacts.txt"));
        MainWindow window(book);
        window.show();
        return app.exec();
    }
    catch (const std::exception &e)
    {
        QMessageBox::critical(nullptr, "Phone book", QString::fromUtf8(e.what()));
        return 1;
    }
}
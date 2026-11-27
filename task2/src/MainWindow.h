#pragma once
#include <QList>
#include <QMainWindow>
#include "PhoneBook.h"

class ContactFilterProxy;
class ContactTableModel;
class QAction;
class QLineEdit;
class QTableView;
class QToolButton;

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(PhoneBook &book, QWidget *parent = nullptr);

private slots:
    void addContact();
    void editContact();
    void deleteContact();
    void applySearch();

private:
    int selectedContactId() const; // -1 if nothing is selected
    void updateStatus();

    PhoneBook &m_book;
    ContactTableModel *m_model;
    ContactFilterProxy *m_proxy;
    QTableView *m_view;
    QLineEdit *m_search;
    QList<QAction *> m_fieldActions;
};
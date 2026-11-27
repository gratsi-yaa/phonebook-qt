#include "MainWindow.h"
#include <QAction>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLineEdit>
#include <QMenu>
#include <QMessageBox>
#include <QPushButton>
#include <QStatusBar>
#include <QTableView>
#include <QToolButton>
#include <QVBoxLayout>
#include "ContactDialog.h"
#include "ContactFilterProxy.h"
#include "ContactTableModel.h"
#include "QtUtil.h"

MainWindow::MainWindow(PhoneBook &book, QWidget *parent) : QMainWindow(parent), m_book(book)
{
    setWindowTitle(tr("Phone book"));
    resize(1150, 600);

    m_model = new ContactTableModel(m_book, this);
    m_proxy = new ContactFilterProxy(this);
    m_proxy->setSourceModel(m_model);
    m_proxy->setSortRole(ContactTableModel::SortRole);
    m_proxy->setSortCaseSensitivity(Qt::CaseInsensitive);

    m_view = new QTableView(this);
    m_view->setModel(m_proxy);
    m_view->setSortingEnabled(true); // click on a header to sort
    m_view->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_view->setSelectionMode(QAbstractItemView::SingleSelection);
    m_view->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_view->verticalHeader()->hide();
    m_view->horizontalHeader()->setStretchLastSection(true);
    m_view->sortByColumn(ContactTableModel::LastName, Qt::AscendingOrder);
    connect(m_view, &QTableView::doubleClicked, this, [this]
            { editContact(); });

    // Search bar: text + choice of the fields to search in
    m_search = new QLineEdit(this);
    m_search->setPlaceholderText(tr("Search..."));
    m_search->setClearButtonEnabled(true);
    connect(m_search, &QLineEdit::textChanged, this, &MainWindow::applySearch);

    auto *fieldsButton = new QToolButton(this);
    fieldsButton->setText(tr("Search in fields"));
    fieldsButton->setPopupMode(QToolButton::InstantPopup);
    auto *menu = new QMenu(fieldsButton);
    for (int col = 0; col < ContactTableModel::ColumnCount; ++col)
    {
        QAction *action = menu->addAction(m_model->headerData(col, Qt::Horizontal).toString());
        action->setCheckable(true);
        action->setChecked(true);
        connect(action, &QAction::toggled, this, &MainWindow::applySearch);
        m_fieldActions.append(action);
    }
    fieldsButton->setMenu(menu);

    auto *addButton = new QPushButton(tr("Add"), this);
    auto *editButton = new QPushButton(tr("Edit"), this);
    auto *deleteButton = new QPushButton(tr("Delete"), this);
    connect(addButton, &QPushButton::clicked, this, &MainWindow::addContact);
    connect(editButton, &QPushButton::clicked, this, &MainWindow::editContact);
    connect(deleteButton, &QPushButton::clicked, this, &MainWindow::deleteContact);

    auto *top = new QHBoxLayout;
    top->addWidget(m_search, 1);
    top->addWidget(fieldsButton);
    top->addSpacing(20);
    top->addWidget(addButton);
    top->addWidget(editButton);
    top->addWidget(deleteButton);

    auto *central = new QWidget(this);
    auto *layout = new QVBoxLayout(central);
    layout->addLayout(top);
    layout->addWidget(m_view);
    setCentralWidget(central);

    updateStatus();
}

int MainWindow::selectedContactId() const
{
    QModelIndex idx = m_view->currentIndex();
    if (!idx.isValid())
        return -1;
    return m_model->contactIdAt(m_proxy->mapToSource(idx).row());
}

void MainWindow::applySearch()
{
    QSet<int> columns;
    for (int i = 0; i < m_fieldActions.size(); ++i)
        if (m_fieldActions[i]->isChecked())
            columns.insert(i);
    m_proxy->setSearch(m_search->text().trimmed(), columns);
    updateStatus();
}

void MainWindow::addContact()
{
    ContactDialog dialog(this);
    if (dialog.exec() != QDialog::Accepted)
        return;
    try
    {
        m_book.add(dialog.contact());
        m_model->reload();
        updateStatus();
    }
    catch (const std::exception &e)
    {
        QMessageBox::critical(this, tr("Error"), QString::fromUtf8(e.what()));
    }
}

void MainWindow::editContact()
{
    const Contact *found = m_book.find(selectedContactId());
    if (!found)
    {
        QMessageBox::information(this, tr("Edit"), tr("Select a contact first."));
        return;
    }
    Contact copy = *found;
    ContactDialog dialog(this, &copy);
    if (dialog.exec() != QDialog::Accepted)
        return;
    try
    {
        m_book.update(dialog.contact());
        m_model->reload();
        updateStatus();
    }
    catch (const std::exception &e)
    {
        QMessageBox::critical(this, tr("Error"), QString::fromUtf8(e.what()));
    }
}

void MainWindow::deleteContact()
{
    int id = selectedContactId();
    const Contact *found = m_book.find(id);
    if (!found)
    {
        QMessageBox::information(this, tr("Delete"), tr("Select a contact first."));
        return;
    }
    QString name = fromStd(found->lastName + " " + found->firstName);
    if (QMessageBox::question(this, tr("Delete"), tr("Delete contact %1?").arg(name)) != QMessageBox::Yes)
        return;
    try
    {
        m_book.remove(id);
        m_model->reload();
        updateStatus();
    }
    catch (const std::exception &e)
    {
        QMessageBox::critical(this, tr("Error"), QString::fromUtf8(e.what()));
    }
}

void MainWindow::updateStatus()
{
    statusBar()->showMessage(tr("Contacts: %1 shown / %2 total")
                                 .arg(m_proxy->rowCount())
                                 .arg(m_book.contacts().size()));
}
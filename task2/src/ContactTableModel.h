#pragma once
#include <QAbstractTableModel>
#include "PhoneBook.h"

// Table model on top of PhoneBook (read-only; edits go through dialogs).
class ContactTableModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    enum Column
    {
        LastName,
        FirstName,
        MiddleName,
        BirthDate,
        Email,
        Phones,
        Address,
        ColumnCount
    };
    static constexpr int SortRole = Qt::UserRole;

    explicit ContactTableModel(PhoneBook &book, QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const override;

    int contactIdAt(int row) const;
    void reload(); // call after the phone book changed

private:
    PhoneBook &m_book;
};
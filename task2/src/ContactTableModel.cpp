#include "ContactTableModel.h"
#include "QtUtil.h"

ContactTableModel::ContactTableModel(PhoneBook &book, QObject *parent)
    : QAbstractTableModel(parent), m_book(book) {}

int ContactTableModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : static_cast<int>(m_book.contacts().size());
}

int ContactTableModel::columnCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : ColumnCount;
}

QVariant ContactTableModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= rowCount())
        return {};
    if (role != Qt::DisplayRole && role != SortRole)
        return {};
    const Contact &c = m_book.contacts()[static_cast<std::size_t>(index.row())];

    switch (index.column())
    {
    case LastName:
        return fromStd(c.lastName);
    case FirstName:
        return fromStd(c.firstName);
    case MiddleName:
        return fromStd(c.middleName);
    case Email:
        return fromStd(c.email);
    case Address:
        return fromStd(c.address);
    case BirthDate:
        if (!c.birthDate)
            return QString();
        if (role == SortRole) // yyyyMMdd sorts chronologically
            return QString::asprintf("%04d%02d%02d", c.birthDate->year,
                                     c.birthDate->month, c.birthDate->day);
        return fromStd(c.birthDate->toString());
    case Phones:
    {
        QStringList parts;
        for (const PhoneNumber &p : c.phones)
            parts << fromStd(p.number) + " (" + QString::fromLatin1(toString(p.type)) + ")";
        return parts.join(", ");
    }
    }
    return {};
}

QVariant ContactTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation != Qt::Horizontal || role != Qt::DisplayRole)
        return {};
    switch (section)
    {
    case LastName:
        return tr("Last name");
    case FirstName:
        return tr("First name");
    case MiddleName:
        return tr("Middle name");
    case BirthDate:
        return tr("Birth date");
    case Email:
        return tr("E-mail");
    case Phones:
        return tr("Phones");
    case Address:
        return tr("Address");
    }
    return {};
}

int ContactTableModel::contactIdAt(int row) const
{
    return m_book.contacts()[static_cast<std::size_t>(row)].id;
}

void ContactTableModel::reload()
{
    beginResetModel();
    endResetModel();
}
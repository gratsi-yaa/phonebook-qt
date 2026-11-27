#include "ContactFilterProxy.h"

void ContactFilterProxy::setSearch(const QString &text, const QSet<int> &columns)
{
    m_text = text;
    m_columns = columns;
    beginFilterChange();
    endFilterChange(QSortFilterProxyModel::Direction::Rows);
}

bool ContactFilterProxy::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    if (m_text.isEmpty())
        return true;
    const int count = sourceModel()->columnCount();
    for (int col = 0; col < count; ++col)
    {
        if (!m_columns.isEmpty() && !m_columns.contains(col))
            continue;
        QModelIndex idx = sourceModel()->index(sourceRow, col, sourceParent);
        if (sourceModel()->data(idx, Qt::DisplayRole).toString().contains(m_text, Qt::CaseInsensitive))
            return true;
    }
    return false;
}
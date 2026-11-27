#pragma once
#include <QSet>
#include <QSortFilterProxyModel>

// Keeps a row if the text is found (case-insensitive) in ANY selected column.
// Sorting by clicking a column header comes from QSortFilterProxyModel.
class ContactFilterProxy : public QSortFilterProxyModel
{
    Q_OBJECT
public:
    using QSortFilterProxyModel::QSortFilterProxyModel;
    void setSearch(const QString &text, const QSet<int> &columns);

protected:
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override;

private:
    QString m_text;
    QSet<int> m_columns; // empty = all columns
};
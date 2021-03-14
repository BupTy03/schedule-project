#pragma once
#include <QSortFilterProxyModel>
#include <QString>
#include <QSet>


class QStringSetFilterProxyModel : public QSortFilterProxyModel
{
public:
    explicit QStringSetFilterProxyModel(QObject* parent = nullptr);

    void addFilterString(const QString& str);
    void removeFilterString(const QString& str);
    void setFilterStringsSet(const QSet<QString>& strSet);
    void clearFilterStringsSet();

protected:
    bool filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const override;

private:
    QSet<QString> strSet_;
};

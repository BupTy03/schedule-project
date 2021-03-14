#include "qstringsetfilterproxymodel.hpp"


QStringSetFilterProxyModel::QStringSetFilterProxyModel(QObject* parent)
    : QSortFilterProxyModel(parent)
{
}

void QStringSetFilterProxyModel::addFilterString(const QString& str)
{
    strSet_.insert(str);
    invalidateFilter();
}

void QStringSetFilterProxyModel::removeFilterString(const QString& str)
{
    strSet_.remove(str);
    invalidateFilter();
}

void QStringSetFilterProxyModel::setFilterStringsSet(const QSet<QString>& strSet)
{
    strSet_ = strSet;
    invalidateFilter();
}

void QStringSetFilterProxyModel::clearFilterStringsSet()
{
    strSet_.clear();
    invalidateFilter();
}

bool QStringSetFilterProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const
{
    auto model = sourceModel();
    assert(model != nullptr);

    const QModelIndex index = model->index(sourceRow, filterKeyColumn(), sourceParent);
    return !strSet_.contains(index.data().toString());
}

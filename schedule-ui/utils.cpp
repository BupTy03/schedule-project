#include "utils.hpp"

#include <QModelIndex>
#include <QAbstractItemModel>

#include <algorithm>


QPainterBrushOriginSaver::QPainterBrushOriginSaver(QPainter& painter)
    : painter_(&painter)
    , origin_(painter.brushOrigin())
{
}

QPainterBrushOriginSaver::~QPainterBrushOriginSaver()
{
    painter_->setBrushOrigin(origin_);
}


QPainterStateSaver::QPainterStateSaver(QPainter& painter)
    : painter_(&painter)
{
    painter_->save();
}

QPainterStateSaver::~QPainterStateSaver()
{
    painter_->restore();
}


bool QModelIndexGreaterByRow::operator()(const QModelIndex& lhs, const QModelIndex& rhs) const
{
    return lhs.row() > rhs.row();
}


void CopyRows(const QAbstractItemModel& from, QAbstractItemModel& to)
{
    to.removeRows(0, to.rowCount());

    const int rowCount = from.rowCount();
    to.insertRows(0, rowCount);
    for (int row = 0; row < rowCount; ++row)
        to.setData(to.index(row, 0), from.index(row, 0).data());
}

void MoveRows(QAbstractItemModel& from, QAbstractItemModel& to, QList<QModelIndex> rows)
{
    for (const auto& row : rows)
    {
        assert(row.model() == &from);

        const int addedRowIndex = to.rowCount();
        to.insertRow(addedRowIndex);
        to.setData(to.index(addedRowIndex, 0), row.data());
    }

    std::sort(rows.begin(), rows.end(), QModelIndexGreaterByRow());
    for (const auto& row : rows)
        from.removeRow(row.row());
}

QString Join(const std::set<QString>& strSet, const QString& glue)
{
    QStringList lst;
    lst.reserve(strSet.size());
    for(auto&& s : strSet)
        lst.push_back(s);

    return lst.join(glue);
}

#pragma once

#include <QPainter>


class QModelIndex;
class QAbstractItemModel;


struct QPainterBrushOriginSaver
{
    explicit QPainterBrushOriginSaver(QPainter& painter);
    ~QPainterBrushOriginSaver();

    QPainterBrushOriginSaver(const QPainterBrushOriginSaver&) = delete;
    QPainterBrushOriginSaver& operator=(const QPainterBrushOriginSaver&) = delete;

private:
    QPainter* painter_;
    const QPointF origin_;
};


struct QPainterStateSaver
{
    explicit QPainterStateSaver(QPainter& painter);
    ~QPainterStateSaver();

    QPainterStateSaver(const QPainterStateSaver&) = delete;
    QPainterStateSaver& operator=(const QPainterStateSaver&) = delete;

private:
    QPainter* const painter_;
};


struct QModelIndexGreaterByRow
{
    bool operator()(const QModelIndex& lhs, const QModelIndex& rhs) const;
};


void CopyRows(const QAbstractItemModel& from, QAbstractItemModel& to);
void MoveRows(QAbstractItemModel& from, QAbstractItemModel& to, QList<QModelIndex> rows);

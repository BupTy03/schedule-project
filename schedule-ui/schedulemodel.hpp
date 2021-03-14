#pragma once

#include <QAbstractTableModel>
#include <QString>

#include <array>
#include <vector>


static constexpr int MAX_DAYS_PER_WEEK = 6;
static constexpr int MAX_LESSONS_PER_DAY_COUNT = 6;

struct ScheduleItem
{
    int ClassRoom = 0;
    QString Professor;
    QString Subject;
};

QString ToString(const ScheduleItem& item);


using DaySchedule = std::array<ScheduleItem, MAX_LESSONS_PER_DAY_COUNT>;
using GroupSchedule = std::array<DaySchedule, MAX_DAYS_PER_WEEK>;


class ScheduleModel : public QAbstractTableModel
{
public:
    explicit ScheduleModel(QObject* parent = nullptr);
    void setGroups(const std::vector<GroupSchedule>& groups);

public:// QAbstractTableModel interface
    int rowCount(const QModelIndex& parent) const override;
    int columnCount(const QModelIndex& parent) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    QVariant data(const QModelIndex& index, int role) const override;

private:
    std::vector<GroupSchedule> groups_;
};

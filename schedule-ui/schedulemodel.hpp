#pragma once
#include "ScheduleCommon.hpp"

#include <QAbstractTableModel>
#include <QString>

#include <array>
#include <vector>


struct ScheduleModelItem
{
    QString ClassRoom;
    QString Professor;
    QString Subject;
};

QString ToString(const ScheduleModelItem& item);


using DaySchedule = std::array<ScheduleModelItem, MAX_LESSONS_PER_DAY>;
using GroupSchedule = std::pair<QString, std::array<DaySchedule, DAYS_IN_SCHEDULE_WEEK>>;


class ScheduleModel : public QAbstractTableModel
{
public:
    explicit ScheduleModel(QObject* parent = nullptr);
    void setGroups(const std::vector<GroupSchedule>& groups);
    [[nodiscard]] const std::vector<GroupSchedule>& groups() const;

public:// QAbstractTableModel interface
    [[nodiscard]] int rowCount(const QModelIndex& parent) const override;
    [[nodiscard]] int columnCount(const QModelIndex& parent) const override;
    [[nodiscard]] QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    [[nodiscard]] QVariant data(const QModelIndex& index, int role) const override;

private:
    std::vector<GroupSchedule> groups_;
};

#include "schedulemodel.hpp"
#include <QStringBuilder>


QString ToString(const ScheduleModelItem& item)
{
    if (item.ClassRoom == 0 && item.Professor.isEmpty() && item.Subject.isEmpty())
        return {};

    return item.Subject % ' ' % item.Professor % " (" % item.ClassRoom % ')';
}


ScheduleModel::ScheduleModel(QObject* parent)
    : QAbstractTableModel(parent)
{
}

void ScheduleModel::setGroups(const std::vector<GroupSchedule>& groups)
{
    if(!groups_.empty())
    {
        beginRemoveColumns(QModelIndex(), 0, static_cast<int>(groups_.size() - 1));
        groups_.clear();
        endRemoveColumns();
    }

    beginInsertColumns(QModelIndex(), 0, static_cast<int>(groups.size()) - 1);
    groups_ = groups;
    endInsertColumns();
}

int ScheduleModel::rowCount(const QModelIndex& parent) const
{
    (void)parent;
    return MAX_DAYS_PER_WEEK * MAX_LESSONS_PER_DAY_COUNT;
}

int ScheduleModel::columnCount(const QModelIndex& parent) const
{
    (void)parent;
    return static_cast<int>(groups_.size());
}

QVariant ScheduleModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::ItemDataRole::DisplayRole)
        return QAbstractTableModel::headerData(section, orientation, role);

    if (Qt::Orientation::Horizontal == orientation)
        return groups_.at(section).first;

    // Qt::Orientation::Vertical == orientation
    return section % MAX_LESSONS_PER_DAY_COUNT + 1;
}

QVariant ScheduleModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid())
        return {};

    switch (static_cast<Qt::ItemDataRole>(role))
    {
    case Qt::ItemDataRole::DisplayRole:
    {
        const auto day = static_cast<std::size_t>(index.row() / MAX_DAYS_PER_WEEK);
        const auto group = static_cast<std::size_t>(index.column());
        const auto subject = static_cast<std::size_t>(index.row() % MAX_LESSONS_PER_DAY_COUNT);
        return ToString(groups_.at(group).second.at(day).at(subject));
    }
    default:
        return {};
    }
}

const std::vector<GroupSchedule>& ScheduleModel::groups() const
{
    return groups_;
}

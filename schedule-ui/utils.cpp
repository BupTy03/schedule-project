#include "utils.hpp"

#include <QModelIndex>
#include <QAbstractItemModel>

#include <map>
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

std::set<std::size_t> IndexesSet(std::size_t count)
{
    std::set<std::size_t> result;
    for(std::size_t i = 0; i < count; ++i)
        result.emplace(i);

    return result;
}

std::set<WeekDay> ToWeekDaysSet(WeekDaysType weekDays)
{
    std::set<WeekDay> result;
    for (std::size_t w = 0; w < weekDays.size(); ++w)
    {
        if (weekDays.at(w))
            result.emplace(static_cast<WeekDay>(w));
    }

    if(result.empty()) {
        return std::set<WeekDay>{
                WeekDay::Monday,
                WeekDay::Tuesday,
                WeekDay::Wednesday,
                WeekDay::Thursday,
                WeekDay::Friday,
                WeekDay::Saturday
        };
    }

    return result;
}

std::set<std::size_t> ToGroupsSet(const QStringList& allGroups, const QStringList& currentGroups)
{
    if(currentGroups.empty())
        return IndexesSet(allGroups.size());

    std::set<std::size_t> result;
    for(auto&& g : currentGroups)
    {
        const auto groupIndex = allGroups.indexOf(g);
        assert(groupIndex >= 0);
        if(groupIndex < 0)
            continue;

        result.emplace(groupIndex);
    }

    return result;
}

std::set<std::size_t> ToClassroomsSet(const QStringList& allClassrooms, const ClassroomsSet& currentClassrooms)
{
    std::set<std::size_t> result;
    if(currentClassrooms.empty())
        return IndexesSet(allClassrooms.size());

    for(auto&& c : currentClassrooms)
    {
        const auto classroomIndex = allClassrooms.indexOf(c);
        assert(classroomIndex >= 0);
        if(classroomIndex < 0)
            continue;

        result.emplace(classroomIndex);
    }

    return result;
}

QString ToWarningMessage(ScheduleDataValidationResult validationResult)
{
    static const std::map<ScheduleDataValidationResult, QString> mapping = {
            {ScheduleDataValidationResult::Ok, QObject::tr("Ок")},
            {ScheduleDataValidationResult::ToFewLessonsPerDayRequested, QObject::tr("Слишком мало пар в день")},
            {ScheduleDataValidationResult::NoGroups, QObject::tr("Необходимо добавить хотя бы одну группу")},
            {ScheduleDataValidationResult::NoSubjects, QObject::tr("Необходимо добавить хотя бы одну дисциплину")},
            {ScheduleDataValidationResult::NoProfessors, QObject::tr("Необходимо добавить хотя бы одного преподавателя")},
            {ScheduleDataValidationResult::NoClassrooms, QObject::tr("Необходимо добавить хотя бы одну аудиторию")}
    };

    auto it = mapping.find(validationResult);
    if(it == mapping.end())
    {
        assert(false && "Unknown enum value");
        return QObject::tr("Неизвестная ошибка");
    }

    return it->second;
}

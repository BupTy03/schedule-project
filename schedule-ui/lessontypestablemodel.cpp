#include "lessontypestablemodel.hpp"
#include "utils.hpp"
#include "scheduleimportexport.hpp"

#include <map>
#include <numeric>


static constexpr auto DEFAULT_COLUMNS_COUNT = 6;

enum class ColumnType
{
    Name,
    Groups,
    Hours,
    Days,
    Classrooms,
    Complexity
};


LessonTypesTableModel::LessonTypesTableModel(QObject* parent)
    : QAbstractTableModel(parent)
    , lessons_()
{
    lessons_.reserve(3);
    lessons_.emplace_back(tr("Лекция"),
                          StringsSet(),
                          0,
                          0,
                          WeekDays(),
                          StringsSet());

    lessons_.emplace_back(tr("Практика"),
                          StringsSet(),
                          0,
                          0,
                          WeekDays(),
                          StringsSet());

    lessons_.emplace_back(tr("Лабораторная"),
                          StringsSet(),
                          0,
                          0,
                          WeekDays(),
                          StringsSet{});
}

const std::vector<LessonTypeItem>& LessonTypesTableModel::lessons() const
{
    return lessons_;
}

int LessonTypesTableModel::rowCount(const QModelIndex&) const
{
    return static_cast<int>(lessons_.size());
}

int LessonTypesTableModel::columnCount(const QModelIndex&) const
{
    return DEFAULT_COLUMNS_COUNT;
}

QVariant LessonTypesTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (Qt::ItemDataRole::DisplayRole != role)
        return {};

    if (Qt::Orientation::Vertical == orientation)
        return {};

    static const std::array<QString, DEFAULT_COLUMNS_COUNT> sections = {
        tr("Тип"), tr("Группы"), tr("Часов в неделю"), tr("Дни"), tr("Аудитории"), tr("Сложность")
    };
    return sections.at(static_cast<std::size_t>(section));
}

QVariant LessonTypesTableModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid())
        return {};

    const auto& lesson = lessons_.at(static_cast<std::size_t>(index.row()));
    const auto columnType = static_cast<ColumnType>(index.column());

    switch (role)
    {
    case Qt::ItemDataRole::DisplayRole:
    {
        switch (columnType)
        {
        case ColumnType::Name:
            return lesson.Name;
        case ColumnType::Groups:
            return Join(lesson.Groups, ", ");
        case ColumnType::Hours:
            return lesson.CountHoursPerWeek;
        case ColumnType::Days:
            return WeekDaysString(lesson.WeekDaysRequested);
        case ColumnType::Classrooms:
            return Join(lesson.Classrooms, ", ");
        case ColumnType::Complexity:
            return lesson.Complexity;
        }
        break;
    }
    case Qt::ItemDataRole::UserRole:
    {
        if (columnType == ColumnType::Groups)
            return QVariant::fromValue(lesson.Groups);

        if (columnType == ColumnType::Days)
            return QVariant::fromValue(lesson.WeekDaysRequested);

        if(columnType == ColumnType::Classrooms)
            return QVariant::fromValue(lesson.Classrooms);

        break;
    }
    case Qt::ItemDataRole::TextAlignmentRole:
    {
        switch (columnType)
        {
        case ColumnType::Name:
        case ColumnType::Hours:
        case ColumnType::Complexity:
            return Qt::AlignmentFlag::AlignCenter;
        case ColumnType::Groups:
        case ColumnType::Days:
        case ColumnType::Classrooms:
            return static_cast<int>(Qt::AlignmentFlag::AlignVCenter | Qt::AlignmentFlag::AlignLeft);
        }
        break;
    }
    }

    return {};
}

bool LessonTypesTableModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if (!index.isValid())
        return false;

    if (Qt::ItemDataRole::EditRole != role)
        return false;

    auto& lesson = lessons_.at(static_cast<std::size_t>(index.row()));
    switch (static_cast<ColumnType>(index.column()))
    {
    case ColumnType::Name:
        lesson.Name = value.toString();
        break;
    case ColumnType::Hours:
        lesson.CountHoursPerWeek = value.toInt();
        break;
    case ColumnType::Days:
        lesson.WeekDaysRequested = value.value<WeekDays>();
        break;
    case ColumnType::Classrooms:
        lesson.Classrooms = value.value<StringsSet>();
        break;
    case ColumnType::Complexity:
        lesson.Complexity = value.toInt();
        break;
    }

    return true;
}

Qt::ItemFlags LessonTypesTableModel::flags(const QModelIndex& index) const
{
    if (!index.isValid())
        return Qt::ItemFlag::NoItemFlags;

    if (index.column() == 0)
        return Qt::ItemFlag::ItemIsEnabled | Qt::ItemFlag::ItemIsSelectable;

    return Qt::ItemFlag::ItemIsEnabled | Qt::ItemFlag::ItemIsSelectable | Qt::ItemFlag::ItemIsEditable;
}

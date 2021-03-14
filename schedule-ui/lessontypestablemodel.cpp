#include "lessontypestablemodel.hpp"


static constexpr auto DEFAULT_COLUMNS_COUNT = 3;

enum class ColumnType
{
    Name,
    Hours,
    Days
};


QString WeekDaysString(const WeekDaysType& weekDays)
{
    static const std::array<QString, 6> daysNames = {
        QObject::tr("ПН"),
        QObject::tr("ВТ"),
        QObject::tr("СР"),
        QObject::tr("ЧТ"),
        QObject::tr("ПТ"),
        QObject::tr("СБ")
    };

    QString daysStr;
    for (std::size_t d = 0; d < weekDays.size(); ++d)
    {
        if (weekDays.at(d))
        {
            if (!daysStr.isEmpty())
                daysStr.push_back(", ");

            daysStr.push_back(daysNames.at(d));
        }
    }

    return daysStr;
}

QString ToString(const LessonTypeItem& lesson)
{
    return QString("%1 (%2) %3")
            .arg(lesson.Name)
            .arg(lesson.CountHoursPerWeek)
            .arg(WeekDaysString(lesson.WeekDays));
}


LessonTypesTableModel::LessonTypesTableModel(QObject* parent)
    : QAbstractTableModel(parent)
    , lessons_()
{
    lessons_.emplace_back(tr("Лекции"), 0, WeekDaysType{ true, true, true, true, true, true });
    lessons_.emplace_back(tr("Практики"), 0, WeekDaysType{ true, true, true, true, true, true });
    lessons_.emplace_back(tr("Лабораторные"), 0, WeekDaysType{ true, true, true, true, true, true });
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
        tr("Тип"), tr("Часов в неделю"), tr("Дни")
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
        case ColumnType::Hours:
            return lesson.CountHoursPerWeek;
        case ColumnType::Days:
            return WeekDaysString(lesson.WeekDays);
        }
        break;
    }
    case Qt::ItemDataRole::UserRole:
    {
        if (columnType == ColumnType::Days)
            return QVariant::fromValue(lesson.WeekDays);

        break;
    }
    case Qt::ItemDataRole::TextAlignmentRole:
    {
        switch (columnType)
        {
        case ColumnType::Name:
            return static_cast<int>(Qt::AlignmentFlag::AlignVCenter | Qt::AlignmentFlag::AlignLeft);
        case ColumnType::Hours:
            return Qt::AlignmentFlag::AlignCenter;
        case ColumnType::Days:
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
        lesson.WeekDays = value.value<WeekDaysType>();
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

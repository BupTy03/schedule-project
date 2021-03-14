#include "disciplinesmodel.hpp"


static const auto DEFAULT_COLUMNS_COUNT = 4;

enum class ColumnType
{
    Name,
    Professor,
    Groups,
    Lessons
};


static QStringList ToStringList(const std::vector<LessonTypeItem>& lessons)
{
    QStringList result;
    result.reserve(static_cast<int>(lessons.size()));
    for (auto&& lesson : lessons)
        result.push_back(ToString(lesson));

    return result;
}


DisciplinesModel::DisciplinesModel(QObject* parent)
    : QAbstractTableModel(parent)
{
}

void DisciplinesModel::addDiscipline(const Discipline& discipline)
{
    const int rowNum = static_cast<int>(disciplines_.size());
    beginInsertRows(QModelIndex(), rowNum, rowNum);
    disciplines_.emplace_back(discipline);
    endInsertRows();
}

void DisciplinesModel::setDisciplines(const std::vector<Discipline>& disciplines)
{
    if(disciplines.empty())
        return;

    removeRows(0, rowCount(QModelIndex()), QModelIndex());
    beginInsertRows(QModelIndex(), 0, static_cast<int>(disciplines.size()) - 1);
    disciplines_ = disciplines;
    endInsertRows();
}

const std::vector<Discipline>& DisciplinesModel::disciplines() const
{
    return disciplines_;
}

int DisciplinesModel::rowCount(const QModelIndex& parent) const
{
    (void)parent;
    return static_cast<int>(disciplines_.size());
}

int DisciplinesModel::columnCount(const QModelIndex& parent) const
{
    (void)parent;
    return DEFAULT_COLUMNS_COUNT;
}

QVariant DisciplinesModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (Qt::ItemDataRole::DisplayRole != role)
        return {};

    if (Qt::Orientation::Vertical == orientation)
        return {};

    static const std::array<QString, DEFAULT_COLUMNS_COUNT> sections = {
        tr("Дисциплина"), tr("Преподаватель"), tr("Группы"), tr("Занятия")
    };
    return sections.at(static_cast<std::size_t>(section));
}

QVariant DisciplinesModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid())
        return {};

    const auto& discipline = disciplines_.at(static_cast<std::size_t>(index.row()));
    const auto columnType = static_cast<ColumnType>(index.column());

    switch (role)
    {
    case Qt::ItemDataRole::DisplayRole:
    {
        switch (columnType)
        {
        case ColumnType::Name:
            return discipline.Name;
        case ColumnType::Professor:
            return discipline.Professor;
        case ColumnType::Groups:
            return discipline.Groups.join('\n');
        case ColumnType::Lessons:
            return ToStringList(discipline.Lessons).join('\n');
        }
        break;
    }
    }

    return {};
}

bool DisciplinesModel::removeRows(int row, int count, const QModelIndex& parent)
{
    if (count <= 0)
        return true;

    const int countDisciplines = static_cast<int>(disciplines_.size());
    if (row < 0 || row >= countDisciplines || row + count > countDisciplines)
        return false;

    beginRemoveRows(parent, row, row + count - 1);
    disciplines_.erase(disciplines_.begin() + row, disciplines_.begin() + row + count);
    endRemoveRows();
    return true;
}

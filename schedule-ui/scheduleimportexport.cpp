#include "scheduleimportexport.hpp"

#include <QFile>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>

#include <set>


static QStringList ToStringList(const QJsonArray& arr)
{
    QStringList result;
    result.reserve(arr.size());
    for (auto&& val : arr)
        result.push_back(val.toString());

    return result;
}

static QStringList RemoveDuplicatesAndSort(const QStringList& lst)
{
    QStringList result;
    result.reserve(lst.size());

    for (auto&& s : std::set<QString>(lst.begin(), lst.end()))
        result.push_back(s);

    return result;
}

static WeekDaysType ParseWeekDays(const QJsonArray& weekDays)
{
    WeekDaysType result = { false };
    if (weekDays.size() != result.size())
        return result;

    for (std::size_t d = 0; d < result.size(); ++d)
    {
        const auto weekDay = weekDays.at(static_cast<int>(d));
        result.at(d) = weekDay.isBool() && weekDay.toBool(false);
    }

    return result;
}

static LessonTypeItem ParseLesson(const QJsonObject& lesson)
{
    LessonTypeItem res;
    res.Name = lesson["name"].toString();
    res.CountHoursPerWeek = lesson["hours"].toInt(0);
    res.WeekDays = ParseWeekDays(lesson["weekdays"].toArray());
    return res;
}

static std::vector<LessonTypeItem> ParseLessons(const QJsonArray& lessons)
{
    std::vector<LessonTypeItem> result;
    result.reserve(static_cast<std::size_t>(lessons.size()));

    for (auto&& lesson : lessons)
    {
        if (lesson.isObject())
            result.emplace_back(ParseLesson(lesson.toObject()));
    }

    return result;
}

static Discipline ParseDiscipline(const QJsonObject& discipline)
{
    Discipline res;
    res.Name = discipline["name"].toString();
    res.Professor = discipline["professor"].toString();
    res.Groups = ToStringList(discipline["groups"].toArray());
    res.Lessons = ParseLessons(discipline["lessons"].toArray());
    return res;
}

static std::vector<Discipline> ParseDisciplines(const QJsonArray& disciplines)
{
    std::vector<Discipline> result;
    result.reserve(static_cast<std::size_t>(disciplines.size()));
    for (auto&& discipline : disciplines)
    {
        if (discipline.isObject())
            result.emplace_back(ParseDiscipline(discipline.toObject()));
    }

    return result;
}

static QJsonArray ToJson(const WeekDaysType& weekDays)
{
    QJsonArray result;
    for (const auto d : weekDays)
        result.push_back(d);

    return result;
}

static QJsonObject ToJson(const LessonTypeItem& lesson)
{
    return QJsonObject({ { "name", lesson.Name },
                         { "hours", lesson.CountHoursPerWeek },
                         { "weekdays", ToJson(lesson.WeekDays) } });
}

static QJsonArray ToJson(const std::vector<LessonTypeItem>& lessons)
{
    QJsonArray result;
    for (auto&& lesson : lessons)
        result.push_back(ToJson(lesson));

    return result;
}

static QJsonObject ToJson(const Discipline& discipline)
{
    return QJsonObject({ { "name", discipline.Name },
                         { "professor", discipline.Professor },
                         { "groups", QJsonArray::fromStringList(discipline.Groups) },
                         { "lessons", ToJson(discipline.Lessons) } });
}

static QJsonArray ToJson(const std::vector<Discipline>& disciplines)
{
    QJsonArray result;
    for (auto&& discipline : disciplines)
        result.push_back(ToJson(discipline));

    return result;
}


ScheduleDataStorage::~ScheduleDataStorage()
{
}

ScheduleDataJsonFile::ScheduleDataJsonFile(QString filename)
    : filename_(std::move(filename))
    , groups_()
    , professors_()
    , disciplines_()
{
    QFile file(filename_);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        qWarning("Couldn't open json file.");
        return;
    }

    const auto document = QJsonDocument::fromJson(file.readAll());
    if (!document.isObject())
        return;

    const auto docObject = document.object();
    groups_ = RemoveDuplicatesAndSort(ToStringList(docObject["groups"].toArray()));
    professors_ = RemoveDuplicatesAndSort(ToStringList(docObject["professors"].toArray()));
    classrooms_ = RemoveDuplicatesAndSort(ToStringList(docObject["classrooms"].toArray()));
    disciplines_ = ParseDisciplines(docObject["disciplines"].toArray());
}

ScheduleDataJsonFile::~ScheduleDataJsonFile()
{
    QFile file(filename_);
    if (!file.open(QIODevice::OpenModeFlag::WriteOnly | QIODevice::OpenModeFlag::Text))
    {
        qWarning("Couldn't open json file.");
        return;
    }

    QJsonDocument document(QJsonObject({ { "groups", QJsonArray::fromStringList(groups_) },
                                         { "professors", QJsonArray::fromStringList(professors_) },
                                         { "classrooms", QJsonArray::fromStringList(classrooms_) },
                                         { "disciplines", ToJson(disciplines_) } }));

    file.write(document.toJson(QJsonDocument::JsonFormat::Indented));
}

QStringList ScheduleDataJsonFile::groups() const
{
    return groups_;
}

void ScheduleDataJsonFile::saveGroups(const QStringList& groups)
{
    groups_ = RemoveDuplicatesAndSort(groups);
}

QStringList ScheduleDataJsonFile::professors() const
{
    return professors_;
}

void ScheduleDataJsonFile::saveProfessors(const QStringList& professors)
{
    professors_ = RemoveDuplicatesAndSort(professors);
}

QStringList ScheduleDataJsonFile::classrooms() const
{
    return classrooms_;
}

void ScheduleDataJsonFile::saveClassrooms(const QStringList& classrooms)
{
    classrooms_ = classrooms;
}

std::vector<Discipline> ScheduleDataJsonFile::disciplines() const
{
    return disciplines_;
}

void ScheduleDataJsonFile::saveDisciplines(const std::vector<Discipline>& disciplines)
{
    disciplines_ = disciplines;
}

#include "scheduleimportexport.hpp"
#include "utils.hpp"

#include <QFile>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>

#include <set>
#include <algorithm>


LessonTypeItem::LessonTypeItem()
        : Name()
        , CountHoursPerWeek(0)
        , Complexity(0)
        , WeekDaysRequested()
        , Classrooms()
{
}

LessonTypeItem::LessonTypeItem(QString name, int countHoursPerWeek, int complexity, WeekDays weekDays, ClassroomsSet classroomsSet)
        : Name(std::move(name))
        , CountHoursPerWeek(countHoursPerWeek)
        , Complexity(complexity)
        , WeekDaysRequested(weekDays)
        , Classrooms(std::move(classroomsSet))
{
}

bool IsValid(const LessonTypeItem& item, const QStringList& allClassrooms)
{
    return !item.Name.isEmpty() &&
    item.CountHoursPerWeek >= 0 && std::all_of(item.Classrooms.begin(), item.Classrooms.end(),
                                               [&](auto&& classroom){ return allClassrooms.contains(classroom); });
}


QString WeekDaysString(const WeekDays& weekDays)
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
    for (std::size_t d = 0; d < daysNames.size(); ++d)
    {
        if (weekDays.Contains(static_cast<WeekDay>(d)))
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
    return QString("%1 (%2) %3 [%4] %5")
            .arg(lesson.Name)
            .arg(lesson.CountHoursPerWeek)
            .arg(WeekDaysString(lesson.WeekDaysRequested))
            .arg(Join(lesson.Classrooms, ", "))
            .arg(QString(lesson.Complexity, '*'));
}

DisciplineValidationResult Validate(const Discipline& discipline)
{
    if(discipline.Professor.isEmpty())
        return DisciplineValidationResult::NoProfessor;

    if(discipline.Name.isEmpty())
        return DisciplineValidationResult::NoName;

    if(HoursPerWeekSum(discipline.Lessons) <= 0)
        return DisciplineValidationResult::NoLessons;

    if(discipline.Groups.empty())
        return DisciplineValidationResult::NoGroups;

    return DisciplineValidationResult::Ok;
}

static bool Contains(const QStringList& allItems, const QStringList& items)
{
    return std::all_of(items.begin(), items.end(), [&](auto&& item){ return allItems.contains(item);});
}

DisciplineValidationResult Validate(const Discipline& discipline,
                                    const QStringList& allGroups,
                                    const QStringList& allProfessors,
                                    const QStringList& allClassrooms)
{
    const auto validationResult = Validate(discipline);
    if(validationResult != DisciplineValidationResult::Ok)
        return validationResult;

    if(!allProfessors.contains(discipline.Professor))
        return DisciplineValidationResult::ProfessorNotFoundInCommonList;

    if(!Contains(allGroups, discipline.Groups))
        return DisciplineValidationResult::GroupsNotFoundInCommonList;

    if(!std::all_of(discipline.Lessons.begin(), discipline.Lessons.end(), [&](auto&& l){return IsValid(l, allClassrooms);}))
        return DisciplineValidationResult::InvalidLessonItems;

    return DisciplineValidationResult::Ok;
}

QString ToWarningMessage(DisciplineValidationResult validationResult)
{
    static const std::map<DisciplineValidationResult, QString> mapping = {
            {DisciplineValidationResult::Ok, QObject::tr("Ok")},
            {DisciplineValidationResult::NoName, QObject::tr("Необходимо указать название дисциплины")},
            {DisciplineValidationResult::NoProfessor, QObject::tr("Необходимо выбрать преподавателя")},
            {DisciplineValidationResult::NoGroups, QObject::tr("Необходимо выбрать группы")},
            {DisciplineValidationResult::NoLessons, QObject::tr("Необходимо назначить часы для хотя бы одного типа занятий")}
    };

    auto it = mapping.find(validationResult);
    if(it == mapping.end())
    {
        assert(false && "Unknown enum value");
        return QObject::tr("Неизвестная ошибка");
    }

    return it->second;
}

int HoursPerWeekSum(const std::vector<LessonTypeItem>& lessons)
{
    return std::accumulate(lessons.begin(), lessons.end(), 0, [](int lhs, const LessonTypeItem& rhs){
        return lhs + rhs.CountHoursPerWeek;
    });
}

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

    for (auto&& s : SortedSet<QString>(lst.begin(), lst.end()))
        result.push_back(s);

    return result;
}

static WeekDays ParseWeekDays(const QJsonArray& weekDays)
{
    WeekDays result;
    if (weekDays.size() != result.size())
        return result;

    for (std::size_t d = 0; d < result.size(); ++d)
    {
        const auto weekDay = weekDays.at(static_cast<int>(d));
        if(weekDay.isBool() && weekDay.toBool(false))
            result.Add(static_cast<WeekDay>(d));
        else
            result.Remove(static_cast<WeekDay>(d));
    }

    return result;
}

static SortedSet<QString> ParseStringSet(const QJsonArray& strSet)
{
    SortedSet<QString> result;
    for(auto&& str : strSet)
        result.Add(str.toString());

    return result;
}

static LessonTypeItem ParseLesson(const QJsonObject& lesson)
{
    LessonTypeItem res;
    res.Name = lesson["name"].toString();
    res.CountHoursPerWeek = lesson["hours"].toInt(0);
    res.Complexity = lesson["complexity"].toInt(0);
    res.WeekDaysRequested = ParseWeekDays(lesson["weekdays"].toArray());
    res.Classrooms = ParseStringSet(lesson["classrooms"].toArray());
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

static QJsonArray ToJson(const WeekDays& weekDays)
{
    QJsonArray result;
    for (const auto d : weekDays)
        result.push_back(d);

    return result;
}

static QJsonArray ToJson(const SortedSet<QString>& strSet)
{
    QJsonArray result;
    for(auto&& str : strSet)
        result.push_back(str);

    return result;
}

static QJsonObject ToJson(const LessonTypeItem& lesson)
{
    return QJsonObject({ { "name", lesson.Name },
                         { "hours", lesson.CountHoursPerWeek },
                         { "complexity", lesson.Complexity },
                         { "weekdays", ToJson(lesson.WeekDaysRequested) },
                         {"classrooms", ToJson(lesson.Classrooms)}});
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
    classrooms_ = RemoveDuplicatesAndSort(classrooms);
}

std::vector<Discipline> ScheduleDataJsonFile::disciplines() const
{
    return disciplines_;
}

void ScheduleDataJsonFile::saveDisciplines(const std::vector<Discipline>& disciplines)
{
    disciplines_ = disciplines;
}

bool ScheduleDataJsonFile::IsValid() const
{
    return std::all_of(disciplines_.begin(), disciplines_.end(), [&](const Discipline& d){
        return ::Validate(d, groups_, professors_, classrooms_) == DisciplineValidationResult::Ok;
    });
}

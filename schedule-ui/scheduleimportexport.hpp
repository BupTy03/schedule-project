#pragma once
#include "ScheduleCommon.hpp"
#include <QString>
#include <QMetaType>
#include <QStringList>

#include <set>
#include <array>
#include <vector>


using ClassroomsSet = std::set<QString>;

Q_DECLARE_METATYPE(WeekDays);
Q_DECLARE_METATYPE(ClassroomsSet);


struct LessonTypeItem
{
    LessonTypeItem();
    explicit LessonTypeItem(QString name,
                            int countHoursPerWeek,
                            int complexity,
                            WeekDays weekDays,
                            ClassroomsSet classroomsSet);

    QString Name;
    int CountHoursPerWeek;
    int Complexity;
    WeekDays WeekDaysRequested;
    ClassroomsSet Classrooms;
};

bool IsValid(const LessonTypeItem& item, const QStringList& allClassrooms);
QString WeekDaysString(const WeekDays& weekDays);
QString ToString(const LessonTypeItem& lesson);


struct Discipline
{
    QString Name;
    QString Professor;
    QStringList Groups;
    std::vector<LessonTypeItem> Lessons;
};

enum class DisciplineValidationResult
{
    Ok,
    NoName,
    NoProfessor,
    NoGroups,
    NoLessons,
    ProfessorNotFoundInCommonList,
    GroupsNotFoundInCommonList,
    InvalidLessonItems
};

int HoursPerWeekSum(const std::vector<LessonTypeItem>& lessons);
DisciplineValidationResult Validate(const Discipline& discipline);
DisciplineValidationResult Validate(const Discipline& discipline,
                                    const QStringList& allGroups,
                                    const QStringList& allProfessors,
                                    const QStringList& allClassrooms);
QString ToWarningMessage(DisciplineValidationResult validationResult);


class ScheduleDataStorage
{
public:
    virtual ~ScheduleDataStorage();

    [[nodiscard]] virtual QStringList groups() const = 0;
    virtual void saveGroups(const QStringList& groups) = 0;

    [[nodiscard]] virtual QStringList professors() const = 0;
    virtual void saveProfessors(const QStringList& professors) = 0;

    [[nodiscard]] virtual QStringList classrooms() const = 0;
    virtual void saveClassrooms(const QStringList& classrooms) = 0;

    [[nodiscard]] virtual std::vector<Discipline> disciplines() const = 0;
    virtual void saveDisciplines(const std::vector<Discipline>& disciplines) = 0;

    [[nodiscard]] virtual bool IsValid() const = 0;
};


class ScheduleDataJsonFile : public ScheduleDataStorage
{
public:
    explicit ScheduleDataJsonFile(QString filename);
    ~ScheduleDataJsonFile() override;

    [[nodiscard]] QStringList groups() const override;
    void saveGroups(const QStringList& groups) override;

    [[nodiscard]] QStringList professors() const override;
    void saveProfessors(const QStringList& professors) override;

    [[nodiscard]] QStringList classrooms() const override;
    void saveClassrooms(const QStringList& classrooms) override;

    [[nodiscard]] std::vector<Discipline> disciplines() const override;
    void saveDisciplines(const std::vector<Discipline>& disciplines) override;

    [[nodiscard]] bool IsValid() const override;

private:
    QString filename_;
    QStringList groups_;
    QStringList professors_;
    QStringList classrooms_;
    std::vector<Discipline> disciplines_;
};


#pragma once
#include <QString>
#include <QMetaType>
#include <QStringList>

#include <array>
#include <vector>


using WeekDaysType = std::array<bool, 6>;

Q_DECLARE_METATYPE(WeekDaysType);


struct LessonTypeItem
{
    LessonTypeItem() = default;
    explicit LessonTypeItem(QString name,
                            int countHoursPerWeek,
                            WeekDaysType weekDays)
        : Name(std::move(name))
        , CountHoursPerWeek(countHoursPerWeek)
        , WeekDays(weekDays)
    {
    }

    QString Name;
    int CountHoursPerWeek;
    WeekDaysType WeekDays;
};

QString WeekDaysString(const WeekDaysType& weekDays);
QString ToString(const LessonTypeItem& lesson);


struct Discipline
{
    QString Name;
    QString Professor;
    QStringList Groups;
    std::vector<LessonTypeItem> Lessons;
};


class ScheduleDataStorage
{
public:
    virtual ~ScheduleDataStorage();

    virtual QStringList groups() const = 0;
    virtual void saveGroups(const QStringList& groups) = 0;

    virtual QStringList professors() const = 0;
    virtual void saveProfessors(const QStringList& professors) = 0;

    virtual QStringList classrooms() const = 0;
    virtual void saveClassrooms(const QStringList& classrooms) = 0;

    virtual std::vector<Discipline> disciplines() const = 0;
    virtual void saveDisciplines(const std::vector<Discipline>& disciplines) = 0;
};


class ScheduleDataJsonFile : public ScheduleDataStorage
{
public:
    explicit ScheduleDataJsonFile(QString filename);
    ~ScheduleDataJsonFile() override;

    QStringList groups() const override;
    void saveGroups(const QStringList& groups) override;

    QStringList professors() const override;
    void saveProfessors(const QStringList& professors) override;

    QStringList classrooms() const override;
    void saveClassrooms(const QStringList& classrooms) override;

    std::vector<Discipline> disciplines() const override;
    void saveDisciplines(const std::vector<Discipline>& disciplines) override;

private:
    QString filename_;
    QStringList groups_;
    QStringList professors_;
    QStringList classrooms_;
    std::vector<Discipline> disciplines_;
};


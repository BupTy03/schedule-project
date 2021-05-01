#include "showscheduledialog.hpp"
#include "ui_showscheduledialog.h"
#include "schedulemodel.hpp"
#include "hierarchicalheaderview.hpp"

#include <QtWidgets/QPushButton>
#include <QtWidgets/QTableView>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QHeaderView>

#include <QStringListModel>
#include <QStandardItem>
#include <QStandardItemModel>

#include <QFileDialog>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>

#include <array>


static std::unique_ptr<QStandardItemModel> BuildWeekDayHeaderModel()
{
    auto result = std::make_unique<QStandardItemModel>();

    const std::array<QString, 6> days = {
        "Понедельник",
        "Вторник",
        "Среда",
        "Четверг",
        "Пятница",
        "Суббота"
    };

    for (std::size_t i = 0; i < days.size(); ++i)
    {
        QStandardItem* rotatedTextCell = new QStandardItem(days.at(i));
        rotatedTextCell->setSizeHint(QSize(rotatedTextCell->sizeHint().width() + 30, rotatedTextCell->sizeHint().height()));
        rotatedTextCell->setData(1, Qt::UserRole);

        for (int i = 1; i <= MAX_LESSONS_PER_DAY_COUNT; ++i)
        {
            auto item = new QStandardItem(QString::number(i));
            item->setSizeHint(QSize(item->sizeHint().width() + 20, item->sizeHint().height()));
            rotatedTextCell->appendColumn({ item });
        }

        result->setItem(0, static_cast<int>(i), rotatedTextCell);
    }

    return result;
}

static ScheduleModelItem ParseScheduleItem(const QJsonObject& scheduleItem)
{
    ScheduleModelItem result;
    result.ClassRoom = scheduleItem["classroom"].toString();
    result.Professor = scheduleItem["professor"].toString();
    result.Subject = scheduleItem["subject"].toString();
    return result;
}

static DaySchedule ParseDay(const QJsonArray& scheduleItems)
{
    DaySchedule result;
    if(result.size() != scheduleItems.size())
        return {};

    for(std::size_t i = 0; i < scheduleItems.size(); ++i)
        result.at(i) = ParseScheduleItem(scheduleItems.at(i).toObject());

    return result;
}

static std::array<DaySchedule, MAX_DAYS_PER_WEEK> ParseDays(const QJsonArray& days)
{
    std::array<DaySchedule, MAX_DAYS_PER_WEEK> result;
    if(result.size() != days.size())
        return {};

    for(std::size_t i = 0; i < days.size(); ++i)
        result.at(i) = ParseDay(days.at(i).toArray());

    return result;
}

static GroupSchedule ParseGroupSchedule(const QJsonObject& groupSchedule)
{
    GroupSchedule result;
    result.first = groupSchedule["name"].toString();
    result.second = ParseDays(groupSchedule["days"].toArray());
    return result;
}

static std::vector<GroupSchedule> ParseGroupsSchedule(const QJsonArray& groupsSchedule)
{
    std::vector<GroupSchedule> result;
    result.reserve(groupsSchedule.size());
    for(auto&& groupSchedule : groupsSchedule)
    {
        if(groupSchedule.isObject())
            result.emplace_back(ParseGroupSchedule(groupSchedule.toObject()));
    }

    return result;
}

static std::pair<std::vector<GroupSchedule>, std::vector<GroupSchedule>> ParseSchedule(const QJsonObject& schedule)
{
    const auto num = schedule["num"];
    const auto denom = schedule["denom"];
    if(!(num.isArray() && denom.isArray()))
        return {};

    return {ParseGroupsSchedule(num.toArray()), ParseGroupsSchedule(denom.toArray())};
}

static QJsonObject ToJson(const ScheduleModelItem& item)
{
    return QJsonObject({
                               {"classroom", item.ClassRoom},
                               {"professor", item.Professor},
                               {"subject",   item.Subject}
                       });
}

static QJsonArray ToJson(const DaySchedule& day)
{
    QJsonArray result;
    for(auto&& scheduleItem : day)
        result.push_back(ToJson(scheduleItem));
    return result;
}

static QJsonArray ToJson(const std::array<DaySchedule, MAX_DAYS_PER_WEEK>& days)
{
    QJsonArray result;
    for(auto&& day : days)
        result.push_back(ToJson(day));

    return result;
}

static QJsonArray ToJson(const std::vector<GroupSchedule>& groupsSchedule)
{
    QJsonArray result;
    for(auto&& groupSchedule : groupsSchedule)
    {
        result.push_back(QJsonObject({
                                             {"name", groupSchedule.first},
                                             {"days", ToJson(groupSchedule.second)}
                                     }));
    }
    return result;
}


static void SaveSchedule(const std::vector<GroupSchedule>& even,
                         const std::vector<GroupSchedule>& odd,
                         const QString& filePath)
{
    if(filePath.isEmpty())
        return;

    QFile file(filePath);
    if(!file.open(QFile::OpenModeFlag::WriteOnly))
        return;

    file.write(QJsonDocument(QJsonObject({
                                                 {"num",   ToJson(even)},
                                                 {"denom", ToJson(odd)}
                                         })).toJson());
}


ShowScheduleDialog::ShowScheduleDialog(QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::ShowScheduleDialog)
    , evenScheduleModel_()
    , oddScheduleModel_()
{
    ui->setupUi(this);
    setWindowFlags(Qt::Dialog | Qt::WindowTitleHint | Qt::WindowCloseButtonHint | Qt::WindowMinimizeButtonHint | Qt::WindowMaximizeButtonHint);
    setWindowTitle(tr("Расписание"));

    ui->tabWidget->setTabText(0, tr("Числитель"));
    ui->tabWidget->setTabText(1, tr("Знаменатель"));

    auto evenView = new QTableView;
    evenView->setModel(&evenScheduleModel_);

    auto evenWdhw = new HierarchicalHeaderView(Qt::Orientation::Vertical, BuildWeekDayHeaderModel());
    evenView->setVerticalHeader(evenWdhw);

    auto evenLayout = new QGridLayout;
    evenLayout->addWidget(evenView);
    ui->evenTab->setLayout(evenLayout);

    auto oddView = new QTableView;
    oddView->setModel(&oddScheduleModel_);

    auto oddWdhw = new HierarchicalHeaderView(Qt::Orientation::Vertical, BuildWeekDayHeaderModel());
    oddView->setVerticalHeader(oddWdhw);

    auto oddLayout = new QGridLayout;
    oddLayout->addWidget(oddView);
    ui->oddTab->setLayout(oddLayout);

    QObject::connect(ui->saveButton, &QPushButton::clicked, this, &ShowScheduleDialog::OnSaveButtonClicked);
    QObject::connect(ui->openButton, &QPushButton::clicked, this, &ShowScheduleDialog::OnOpenButtonClicked);
    QObject::connect(ui->closeButton, &QPushButton::clicked, this, &QDialog::reject);
}

ShowScheduleDialog::~ShowScheduleDialog()
{
    delete ui;
}

void ShowScheduleDialog::setSchedule(const std::vector<GroupSchedule>& evenSchedule,
                                     const std::vector<GroupSchedule>& oddSchedule)
{
    evenScheduleModel_.setGroups(evenSchedule);
    oddScheduleModel_.setGroups(oddSchedule);
}

void ShowScheduleDialog::OnOpenButtonClicked()
{
    const auto filePath = QFileDialog::getOpenFileName(this,
                                                       tr("Открыть расписание"),
                                                       QString(),
                                                       tr("Json (*.json)"));

    if(filePath.isEmpty())
        return;

    QFile file(filePath);
    if(!file.open(QFile::OpenModeFlag::ReadOnly))
        return;

    const auto jsonDoc = QJsonDocument::fromJson(file.readAll());
    if(!jsonDoc.isObject())
        return;

    const auto schedule = ParseSchedule(jsonDoc.object());
    setModal(false);
    setSchedule(schedule.first, schedule.second);
}

void ShowScheduleDialog::OnSaveButtonClicked()
{
    SaveSchedule(evenScheduleModel_.groups(), oddScheduleModel_.groups(), QFileDialog::getSaveFileName(this,
                                                                    tr("Сохранить расписание"),
                                                                    QString(),
                                                                    tr("Json (*.json)")));
}

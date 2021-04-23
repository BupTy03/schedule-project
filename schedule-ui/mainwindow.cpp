#include "mainwindow.hpp"
#include "./ui_mainwindow.h"

#include "utils.hpp"
#include "showscheduledialog.hpp"
#include "adddisciplinedialog.hpp"
#include "currenttabstrategy.hpp"
#include "disciplinesmodel.hpp"
#include "scheduleprocessor.hpp"

#include "ScheduleGenerator.hpp"
#include "SATScheduleGenerator.hpp"

#include <QFileDialog>
#include <QtWidgets/QToolBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QMessageBox>

#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>


static const auto DEFAULT_LESSONS_PER_DAY_COUNT = 3;


MainWindow::MainWindow(std::unique_ptr<ScheduleDataStorage> scheduleData,
                       QWidget* parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , toolBar_(nullptr)
    , groupsListModel_()
    , professorsListModel_()
    , classroomsListModel_()
    , addDisciplineDialog_(new AddDisciplineDialog(&groupsListModel_, &professorsListModel_, &classroomsListModel_, this))
    , showScheduleDialog_(new ShowScheduleDialog(this))
    , tabStrategy_()
    , scheduleData_(std::move(scheduleData))
    , disciplinesModel_(std::make_unique<DisciplinesModel>())
    , scheduleProcessor_(std::make_unique<ScheduleProcessor>(std::make_unique<SATScheduleGenerator>()))
    , scheduleProcessorThread_(new QThread(this))
    , progressBar_(new QProgressBar(this))
{
    ui->setupUi(this);
    setWindowTitle(tr("Генератор расписаний"));

    addDisciplineDialog_->setModal(true);

    toolBar_ = new QToolBar(this);
    toolBar_->addAction(QString(), [this]() { tabStrategy_->onAddItem(); });
    toolBar_->addAction(QString(), [this]() { tabStrategy_->onRemoveItem(); });
    toolBar_->addSeparator();
    toolBar_->addAction(QIcon(":/img/start.ico"), tr("Сгенерировать расписание"), this, &MainWindow::generateSchedule);
    toolBar_->addAction(QIcon(":/img/eye.ico"), tr("Просмотреть расписание"), this, &MainWindow::viewSchedule);

    onTabChanged(0);
    addToolBar(Qt::ToolBarArea::TopToolBarArea, toolBar_);

    ui->groupsListView->setModel(&groupsListModel_);
    ui->groupsListView->setEditTriggers(QAbstractItemView::EditTrigger::NoEditTriggers);
    ui->groupsListView->setSelectionMode(QAbstractItemView::SelectionMode::ExtendedSelection);

    ui->professorsListView->setModel(&professorsListModel_);
    ui->professorsListView->setEditTriggers(QAbstractItemView::EditTrigger::NoEditTriggers);
    ui->professorsListView->setSelectionMode(QAbstractItemView::SelectionMode::ExtendedSelection);

    ui->classroomsListView->setModel(&classroomsListModel_);
    ui->classroomsListView->setEditTriggers(QAbstractItemView::EditTrigger::NoEditTriggers);
    ui->classroomsListView->setSelectionMode(QAbstractItemView::SelectionMode::ExtendedSelection);

    ui->disciplinesTableView->setModel(disciplinesModel_.get());
    ui->disciplinesTableView->setEditTriggers(QAbstractItemView::EditTrigger::NoEditTriggers);
    ui->disciplinesTableView->setSelectionMode(QAbstractItemView::SelectionMode::ExtendedSelection);
    ui->disciplinesTableView->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeMode::ResizeToContents);
    ui->disciplinesTableView->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeMode::ResizeToContents);

    groupsListModel_.setStringList(scheduleData_->groups());
    professorsListModel_.setStringList(scheduleData_->professors());
    classroomsListModel_.setStringList(scheduleData_->classrooms());
    disciplinesModel_->setDisciplines(scheduleData_->disciplines());

    progressBar_->setRange(0, 100);
    ui->statusbar->addPermanentWidget(progressBar_);
    progressBar_->hide();

    scheduleProcessor_->moveToThread(scheduleProcessorThread_);
    scheduleProcessorThread_->start();

    connect(ui->tabWidget, &QTabWidget::currentChanged, this, &MainWindow::onTabChanged);

    connect(this, &MainWindow::startGeneratingSchedule, scheduleProcessor_.get(), &ScheduleProcessor::start);
    connect(scheduleProcessor_.get(), &ScheduleProcessor::done, this, &MainWindow::onScheduleDone);
}

MainWindow::~MainWindow()
{
    scheduleData_->saveGroups(groupsListModel_.stringList());
    scheduleData_->saveProfessors(professorsListModel_.stringList());
    scheduleData_->saveClassrooms(classroomsListModel_.stringList());
    scheduleData_->saveDisciplines(disciplinesModel_->disciplines());
    scheduleProcessorThread_->quit();
    scheduleProcessorThread_->wait();
    delete ui;
}

void MainWindow::onTabChanged(int current)
{
    switch (static_cast<CurrentTabType>(current))
    {
    case CurrentTabType::Disciplines:
        tabStrategy_ = std::make_unique<DisciplinesTabStrategy>(addDisciplineDialog_, disciplinesModel_.get(), ui->disciplinesTableView);
        break;
    case CurrentTabType::Groups:
        tabStrategy_ = std::make_unique<GeneralTabStrategy>(ui->groupsListView, tr("Добавить группу"), tr("Удалить группу"));
        break;
    case CurrentTabType::Professors:
        tabStrategy_ = std::make_unique<GeneralTabStrategy>(ui->professorsListView, tr("Добавить преподавателя"), tr("Удалить преподавателя"));
        break;
    case CurrentTabType::Classrooms:
        tabStrategy_ = std::make_unique<GeneralTabStrategy>(ui->classroomsListView, tr("Добавить аудиторию"), tr("Удалить аудиторию"));
        break;
    }

    const auto actions = toolBar_->actions();
    auto addAction = actions.at(0);
    addAction->setIcon(tabStrategy_->addActionIcon());
    addAction->setToolTip(tabStrategy_->addActionToolTip());

    auto removeAction = actions.at(1);
    removeAction->setIcon(tabStrategy_->removeActionIcon());
    removeAction->setToolTip(tabStrategy_->removeActionToolTip());
}

void MainWindow::generateSchedule()
{
    const auto groups = groupsListModel_.stringList();
    const auto professors = professorsListModel_.stringList();
    const auto classrooms = classroomsListModel_.stringList();
    const auto disciplines = disciplinesModel_->disciplines();

    std::vector<QString> subjects;
    std::vector<SubjectRequest> subjectRequests;
    for (auto&& discipline : disciplines)
    {
        auto professor = professors.indexOf(discipline.Professor);
        assert(professor >= 0);

        for (auto&& lesson : discipline.Lessons)
        {
            if(lesson.CountHoursPerWeek <= 0)
                continue;

            subjects.emplace_back(discipline.Name + " (" + lesson.Name + ')');
            subjectRequests.emplace_back(professor,
                                         lesson.CountHoursPerWeek,
                                         lesson.Complexity,
                                         lesson.WeekDaysRequested,
                                         ToGroupsSet(groups, lesson.Groups),
                                         ToClassroomsSet(classrooms, lesson.Classrooms));
        }
    }

    auto scheduleData = std::make_shared<ScheduleData>(DEFAULT_LESSONS_PER_DAY_COUNT,
                              static_cast<std::size_t>(groups.size()),
                              static_cast<std::size_t>(professors.size()),
                              static_cast<std::size_t>(classrooms.size()),
                              std::move(subjectRequests),
                              std::vector<LessonAddress>());

    const auto validationResult = Validate(*scheduleData);
    if(validationResult != ScheduleDataValidationResult::Ok)
    {
        QMessageBox::warning(this, tr("Предупреждение"), ToWarningMessage(validationResult));
        return;
    }

    scheduleProcessor_->setData(scheduleData);
    startProcess();
    emit startGeneratingSchedule();
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

void MainWindow::onScheduleDone()
{
    endProcess();

    auto resultSchedule = scheduleProcessor_->result();
    if(resultSchedule->Empty())
        return;

    const auto groups = groupsListModel_.stringList();
    const auto professors = professorsListModel_.stringList();
    const auto classrooms = classroomsListModel_.stringList();
    const auto disciplines = disciplinesModel_->disciplines();

    std::vector<QString> subjects;
    for (auto&& discipline : disciplines)
    {
        for (auto&& lesson : discipline.Lessons)
        {
            if(lesson.CountHoursPerWeek <= 0)
                continue;

            subjects.emplace_back(discipline.Name + " (" + lesson.Name + ')');
        }
    }

    std::vector<GroupSchedule> evenGroupsSchedule;
    std::vector<GroupSchedule> oddGroupsSchedule;
    for (std::size_t g = 0; g < groups.size(); ++g)
    {
        GroupSchedule evenSchedule;
        evenSchedule.first = groups.at(g);
        for (std::size_t d = 0; d < 6; ++d)
        {
            DaySchedule& daySchedule = evenSchedule.second.at(d);
            for (std::size_t l = 0; l < MAX_LESSONS_PER_DAY_COUNT; ++l)
            {
                const ScheduleResult::Lesson resultLesson = resultSchedule->At(g, d, l);
                if (resultLesson)
                {
                    ScheduleModelItem item;
                    item.ClassRoom = classrooms.at(resultLesson->Classroom);
                    item.Professor = professors.at(resultLesson->Professor);
                    item.Subject = subjects.at(resultLesson->Subject);
                    daySchedule.at(l) = item;
                }
                else
                {
                    daySchedule.at(l) = ScheduleModelItem{};
                }
            }
        }

        GroupSchedule oddSchedule;
        oddSchedule.first = groups.at(g);
        for (std::size_t d = 6; d < SCHEDULE_DAYS_COUNT; ++d)
        {
            DaySchedule& daySchedule = oddSchedule.second.at(d - 6);
            for (std::size_t l = 0; l < MAX_LESSONS_PER_DAY_COUNT; ++l)
            {
                const ScheduleResult::Lesson resultLesson = resultSchedule->At(g, d, l);
                if (resultLesson)
                {
                    ScheduleModelItem item;
                    item.ClassRoom = classrooms.at(resultLesson->Classroom);
                    item.Professor = professors.at(resultLesson->Professor);
                    item.Subject = subjects.at(resultLesson->Subject);
                    daySchedule.at(l) = item;
                }
                else
                {
                    daySchedule.at(l) = ScheduleModelItem{};
                }
            }
        }

        evenGroupsSchedule.push_back(evenSchedule);
        oddGroupsSchedule.push_back(oddSchedule);
    }

    showScheduleDialog_->setModal(false);
    showScheduleDialog_->setSchedule(evenGroupsSchedule, oddGroupsSchedule);
    SetShowResultMode(*showScheduleDialog_);
    if(showScheduleDialog_->exec() != QDialog::DialogCode::Accepted)
        return;

    const auto filePath = QFileDialog::getSaveFileName(this,
                                                       tr("Сохранить расписание"),
                                                       QString(),
                                                       tr("Json (*.json)"));

    if(filePath.isEmpty())
        return;

    QFile file(filePath);
    if(!file.open(QFile::OpenModeFlag::WriteOnly))
        return;

    file.write(QJsonDocument(QJsonObject({
                                                 {"num",   ToJson(evenGroupsSchedule)},
                                                 {"denom", ToJson(oddGroupsSchedule)}
                                         })).toJson());
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

void MainWindow::viewSchedule()
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
    showScheduleDialog_->setModal(false);
    showScheduleDialog_->setSchedule(schedule.first, schedule.second);
    SetViewMode(*showScheduleDialog_);
    showScheduleDialog_->show();
}

void MainWindow::lockAllControls(bool flag)
{
    ui->tabWidget->setDisabled(flag);
    toolBar_->setDisabled(flag);
}

void MainWindow::startProcess()
{
    progressBar_->setValue(70);
    progressBar_->show();
    lockAllControls(true);
}

void MainWindow::endProcess()
{
    progressBar_->hide();
    lockAllControls(false);
}

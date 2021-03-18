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

#include <QtWidgets/QToolBar>
#include <QtWidgets/QPushButton>


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
    progressBar_->setValue(50);
    progressBar_->show();

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
                                         ToWeekDaysSet(lesson.WeekDays),
                                         ToGroupsSet(groups, discipline.Groups),
                                         ToClassroomsSet(classrooms, lesson.Classrooms));
        }
    }

    auto scheduleData = std::make_shared<ScheduleData>(DEFAULT_LESSONS_PER_DAY_COUNT,
                              static_cast<std::size_t>(groups.size()),
                              static_cast<std::size_t>(professors.size()),
                              static_cast<std::size_t>(classrooms.size()),
                              std::move(subjectRequests));

    scheduleProcessor_->setData(scheduleData);
    progressBar_->setValue(70);
    emit startGeneratingSchedule();
}

void MainWindow::onScheduleDone()
{
    progressBar_->hide();
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
        auto professor = professors.indexOf(discipline.Professor);
        assert(professor >= 0);

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
                    item.ClassRoom = resultLesson->Classroom;
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
                    item.ClassRoom = resultLesson->Classroom;
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

    showScheduleDialog_->setSchedule(evenGroupsSchedule, oddGroupsSchedule);
    showScheduleDialog_->show();
}

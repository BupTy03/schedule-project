#include "mainwindow.hpp"
#include "./ui_mainwindow.h"

#include "showscheduledialog.hpp"
#include "adddisciplinedialog.hpp"
#include "currenttabstrategy.hpp"
#include "scheduleimportexport.hpp"
#include "disciplinesmodel.hpp"

#include "SATScheduleGenerator.hpp"

#include <QAction>
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
    , addDisciplineDialog_(new AddDisciplineDialog(&groupsListModel_, &professorsListModel_, this))
    , showScheduleDialog_(new ShowScheduleDialog(this))
    , tabStrategy_()
    , scheduleData_(std::move(scheduleData))
    , disciplinesModel_(std::make_unique<DisciplinesModel>())
{
    ui->setupUi(this);
    setWindowTitle(tr("Генератор расписаний"));

    addDisciplineDialog_->setModal(true);

    toolBar_ = new QToolBar(this);
    toolBar_->addAction(QString(), [this]() { tabStrategy_->onAddItem(); });
    toolBar_->addAction(QString(), [this]() { tabStrategy_->onRemoveItem(); });
    toolBar_->addSeparator();
    toolBar_->addAction(QIcon(":/img/start.ico"), tr("Сгенерировать расписание"), this, &MainWindow::genetateSchedule);

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

    connect(ui->tabWidget, &QTabWidget::currentChanged, this, &MainWindow::onTabChanged);
}

MainWindow::~MainWindow()
{
    scheduleData_->saveGroups(groupsListModel_.stringList());
    scheduleData_->saveProfessors(professorsListModel_.stringList());
    scheduleData_->saveClassrooms(classroomsListModel_.stringList());
    scheduleData_->saveDisciplines(disciplinesModel_->disciplines());
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

static std::set<WeekDay> ToWeekDaysSet(WeekDaysType weekDays)
{
    std::set<WeekDay> result;
    for (std::size_t w = 0; w < weekDays.size(); ++w)
    {
        if (weekDays.at(w))
            result.emplace(static_cast<WeekDay>(w));
    }

    return result;
}

void MainWindow::genetateSchedule()
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
            subjects.emplace_back(discipline.Name + " (" + lesson.Name + ')');
            subjectRequests.emplace_back(professor,
                                         lesson.CountHoursPerWeek,
                                         ToWeekDaysSet(lesson.WeekDays),
                                         std::set<std::size_t>({ 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 }));
        }
    }

    ScheduleData scheduleData(DEFAULT_LESSONS_PER_DAY_COUNT,
                              static_cast<std::size_t>(groups.size()),
                              static_cast<std::size_t>(professors.size()),
                              static_cast<std::size_t>(classrooms.size()),
                              std::move(subjectRequests));

    SATScheduleGenerator generator;
    const auto resultSchedule = generator.Genetate(scheduleData);
    if(resultSchedule.Empty())
        return;

    std::vector<GroupSchedule> evenGroupsSchedule;
    std::vector<GroupSchedule> oddGroupsSchedule;
    for (std::size_t g = 0; g < groups.size(); ++g)
    {
        GroupSchedule evenSchedule;
        for (std::size_t d = 0; d < 6; ++d)
        {
            DaySchedule daySchedule = evenSchedule.at(d);
            for (std::size_t l = 0; l < MAX_LESSONS_PER_DAY_COUNT; ++l)
            {
                const ScheduleResult::Lesson resultLesson = resultSchedule.At(g, d, l);
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
        for (std::size_t d = 6; d < SCHEDULE_DAYS_COUNT; ++d)
        {
            DaySchedule& daySchedule = oddSchedule.at(d - 6);
            for (std::size_t l = 0; l < MAX_LESSONS_PER_DAY_COUNT; ++l)
            {
                const ScheduleResult::Lesson resultLesson = resultSchedule.At(g, d, l);
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

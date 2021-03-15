#include "mainwindow.hpp"
#include "./ui_mainwindow.h"

#include "showscheduledialog.hpp"
#include "adddisciplinedialog.hpp"
#include "currenttabstrategy.hpp"
#include "scheduleimportexport.hpp"
#include "disciplinesmodel.hpp"

#include <QAction>
#include <QtWidgets/QToolBar>
#include <QtWidgets/QPushButton>


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

void MainWindow::genetateSchedule()
{
    showScheduleDialog_->show();
}

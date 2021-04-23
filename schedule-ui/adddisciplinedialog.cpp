#include "adddisciplinedialog.hpp"
#include "ui_adddisciplinedialog.h"

#include "lessontypestablemodel.hpp"
#include "chooseitemssetwidget.hpp"
#include "qspinboxdelegate.hpp"
#include "chooseweekdaydelegate.hpp"
#include "chooseitemsdelegate.hpp"
#include "utils.hpp"

#include <QtWidgets/QPushButton>

#include <QKeyEvent>
#include <QAction>
#include <QMessageBox>
#include <QDialogButtonBox>

#include <cassert>


static const auto MIN_HOURS_COUNT = 0;
static const auto MAX_HOURS_COUNT = 100;
static const auto MIN_COMPLEXITY = 0;
static const auto MAX_COMPLEXITY = 4;


AddDisciplineDialog::AddDisciplineDialog(QStringListModel* groupsListModel,
                                         QStringListModel* professorListModel,
                                         QStringListModel* classroomsListModel,
                                         QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::AddDisciplineDialog)
    , professorChoose_(new QComboBox(this))
    , disciplineChoose_(new QComboBox(this))
    , lessonsModel_(std::make_unique<LessonTypesTableModel>())
{
    ui->setupUi(this);

    assert(groupsListModel != nullptr);
    assert(professorListModel != nullptr);
    assert(classroomsListModel != nullptr);

    setWindowTitle(tr("Добавление дисциплины"));
    setWindowFlags(Qt::Dialog | Qt::WindowTitleHint | Qt::WindowCloseButtonHint);

    disciplineChoose_->setEditable(true);

    professorChoose_->setModel(professorListModel);
    ui->formLayout->addRow(tr("&Преподаватель:"), professorChoose_);
    ui->formLayout->addRow(tr("&Дисциплина:"), disciplineChoose_);
    ui->lessonsGroupBox->setTitle(tr("Занятия"));
    ui->lessonsTableView->setModel(lessonsModel_.get());
    ui->lessonsTableView->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeMode::Stretch);
    ui->lessonsTableView->setItemDelegateForColumn(1, new ChooseItemsDelegate(groupsListModel,
                                                                          tr("Выбор групп"),
                                                                          tr("Выберите группы"),
                                                                          this));
    ui->lessonsTableView->setItemDelegateForColumn(2, new QSpinBoxDelegate(MIN_HOURS_COUNT, MAX_HOURS_COUNT, this));
    ui->lessonsTableView->setItemDelegateForColumn(3, new ChooseWeekDayDelegate(this));
    ui->lessonsTableView->setItemDelegateForColumn(4, new ChooseItemsDelegate(classroomsListModel,
                                                                              tr("Выбор аудиторий"),
                                                                              tr("Выберите аудитории"),
                                                                              this));

    ui->lessonsTableView->setItemDelegateForColumn(5, new QSpinBoxDelegate(MIN_COMPLEXITY, MAX_COMPLEXITY, this));
    ui->lessonsTableView->setSelectionMode(QAbstractItemView::SelectionMode::ExtendedSelection);

    auto buttonBox = new QDialogButtonBox(this);
    auto okButton = buttonBox->addButton(QDialogButtonBox::StandardButton::Ok);
    okButton->setText(tr("ОК"));

    auto cancelButton = buttonBox->addButton(QDialogButtonBox::StandardButton::Cancel);
    cancelButton->setText(tr("Отмена"));
    layout()->addWidget(buttonBox);

    connect(okButton, &QPushButton::clicked, this, &AddDisciplineDialog::onOkButtonClicked);
    connect(cancelButton, &QPushButton::clicked, this, &AddDisciplineDialog::reject);
    connect(ui->addLessonButton, &QPushButton::clicked, this, &AddDisciplineDialog::onAddLessonButtonClicked);
    connect(ui->removeLessonButton, &QPushButton::clicked, this, &AddDisciplineDialog::onRemoveLessonButtonClicked);
}

AddDisciplineDialog::~AddDisciplineDialog()
{
    delete ui;
}

Discipline AddDisciplineDialog::discipline() const
{
    Discipline res;
    res.Name = disciplineChoose_->currentText();
    res.Professor = professorChoose_->currentText();
    res.Lessons = lessonsModel_->lessons();
    return res;
}

void AddDisciplineDialog::keyPressEvent(QKeyEvent* evt)
{
    if (evt->key() == Qt::Key_Enter || evt->key() == Qt::Key_Return)
        return;

    QDialog::keyPressEvent(evt);
}

void AddDisciplineDialog::onOkButtonClicked()
{
    const auto validationResult = Validate(discipline());
    if(validationResult == DisciplineValidationResult::Ok)
        accept();
    else
        QMessageBox::warning(this, tr("Предупреждение"), ToWarningMessage(validationResult));
}

void AddDisciplineDialog::onAddLessonButtonClicked()
{
    assert(lessonsModel_ != nullptr);
    lessonsModel_->addLesson(LessonTypeItem(tr("Лекция"),
                                            StringsSet(),
                                            0,
                                            0,
                                            WeekDays(),
                                            StringsSet()));
}

void AddDisciplineDialog::onRemoveLessonButtonClicked()
{
    assert(lessonsModel_ != nullptr);

    auto selectionModel = ui->lessonsTableView->selectionModel();
    assert(selectionModel != nullptr);

    auto selectedRows = selectionModel->selectedRows();
    std::sort(selectedRows.begin(), selectedRows.end(), QModelIndexGreaterByRow());
    for (const auto& selected : selectedRows)
        lessonsModel_->removeRow(selected.row());
}

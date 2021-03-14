#include "chooseitemssetwidget.hpp"
#include "ui_chooseitemssetwidget.h"

#include "utils.hpp"

#include <QtWidgets/QPushButton>


ChooseItemsSetWidget::ChooseItemsSetWidget(const QAbstractItemModel* sourceModel,
                                           const QString& title,
                                           QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::ChooseItemsSetWidget)
    , sourceModel_(sourceModel)
    , availableItemsModel_()
    , chosenItemsModel_()
{
    assert(sourceModel != nullptr);

    ui->setupUi(this);
    ui->groupBox->setTitle(title);

    ui->availableItemsListView->setModel(&availableItemsModel_);
    ui->availableItemsListView->setEditTriggers(QAbstractItemView::EditTrigger::NoEditTriggers);
    ui->availableItemsListView->setSelectionMode(QAbstractItemView::SelectionMode::ExtendedSelection);

    ui->chosenItemsListView->setModel(&chosenItemsModel_);
    ui->chosenItemsListView->setEditTriggers(QAbstractItemView::EditTrigger::NoEditTriggers);
    ui->chosenItemsListView->setSelectionMode(QAbstractItemView::SelectionMode::ExtendedSelection);

    ui->availableLabel->hide();
    ui->chosenLabel->hide();

    connect(ui->addButton, &QPushButton::clicked, this, &ChooseItemsSetWidget::addItemButtonClicked);
    connect(ui->removeButton, &QPushButton::clicked, this, &ChooseItemsSetWidget::removeItemButtonClicked);
}

ChooseItemsSetWidget::~ChooseItemsSetWidget()
{
    delete ui;
}

void ChooseItemsSetWidget::setAvailableAndChosenTitles(const QString& availableTitle,
                                                       const QString& chosenTitle)
{
    ui->availableLabel->setText(availableTitle);
    ui->chosenLabel->setText(chosenTitle);

    ui->availableLabel->show();
    ui->chosenLabel->show();
}

QStringList ChooseItemsSetWidget::chosenItemsList() const
{
    return chosenItemsModel_.stringList();
}

void ChooseItemsSetWidget::showEvent(QShowEvent* event)
{
    (void)event;
    chosenItemsModel_.removeRows(0, chosenItemsModel_.rowCount());
    CopyRows(*sourceModel_, availableItemsModel_);
}

void ChooseItemsSetWidget::addItemButtonClicked()
{
    auto selectionModel = ui->availableItemsListView->selectionModel();
    assert(selectionModel != nullptr);
    MoveRows(availableItemsModel_, chosenItemsModel_, selectionModel->selectedRows());
}

void ChooseItemsSetWidget::removeItemButtonClicked()
{
    auto selectionModel = ui->chosenItemsListView->selectionModel();
    assert(selectionModel != nullptr);
    MoveRows(chosenItemsModel_, availableItemsModel_, selectionModel->selectedRows());
}

#include "currenttabstrategy.hpp"
#include "disciplinesmodel.hpp"
#include "adddisciplinedialog.hpp"
#include "utils.hpp"

#include <QTableView>

#include <cassert>


CurrentTabStrategy::~CurrentTabStrategy()
{
}

QIcon CurrentTabStrategy::addActionIcon() const
{
    return QIcon(":/img/add.ico");
}

QIcon CurrentTabStrategy::removeActionIcon() const
{
    return QIcon(":/img/remove.ico");
}


GeneralTabStrategy::GeneralTabStrategy(QAbstractItemView* view, QString addActionToolTip, QString removeActionToolTip)
    : view_(view)
    , addActionToolTip_(std::move(addActionToolTip))
    , removeActionToolTip_(std::move(removeActionToolTip))
{
    assert(view_ != nullptr);
}

QString GeneralTabStrategy::addActionToolTip() const
{
    return addActionToolTip_;
}

QString GeneralTabStrategy::removeActionToolTip() const
{
    return removeActionToolTip_;
}

void GeneralTabStrategy::onAddItem()
{
    auto model = view_->model();
    assert(model != nullptr);

    const auto currentRow = model->rowCount();
    model->insertRow(currentRow);
    view_->edit(model->index(currentRow, 0));
}

void GeneralTabStrategy::onRemoveItem()
{
    auto model = view_->model();
    assert(model != nullptr);

    auto selectionModel = view_->selectionModel();
    assert(selectionModel != nullptr);

    auto selectedRows = selectionModel->selectedRows();
    std::sort(selectedRows.begin(), selectedRows.end(), QModelIndexGreaterByRow());
    for (const auto& selected : selectedRows)
        model->removeRow(selected.row());
}


DisciplinesTabStrategy::DisciplinesTabStrategy(AddDisciplineDialog* addDisciplineWidget,
                                               DisciplinesModel* disciplinesModel,
                                               QTableView* disciplinesView)
    : addDisciplineWidget_(addDisciplineWidget)
    , disciplinesModel_(disciplinesModel)
    , disciplinesView_(disciplinesView)
{
    assert(addDisciplineWidget_ != nullptr);
    assert(disciplinesModel_ != nullptr);
    assert(disciplinesView_ != nullptr);
}

QString DisciplinesTabStrategy::addActionToolTip() const
{
    return QObject::tr("Добавить дисциплину");
}

QString DisciplinesTabStrategy::removeActionToolTip() const
{
    return QObject::tr("Удалить дисциплину");
}

void DisciplinesTabStrategy::onAddItem()
{
    if (addDisciplineWidget_->exec() != QDialog::DialogCode::Accepted)
        return;

    disciplinesModel_->addDiscipline(addDisciplineWidget_->discipline());
}

void DisciplinesTabStrategy::onRemoveItem()
{
    auto selectionModel = disciplinesView_->selectionModel();
    assert(selectionModel != nullptr);

    auto selectedRows = selectionModel->selectedRows();
    std::sort(selectedRows.begin(), selectedRows.end(), QModelIndexGreaterByRow());
    for (const auto& selected : selectedRows)
        disciplinesModel_->removeRow(selected.row());
}

#include "chooseclassroomsdelegate.hpp"
#include "chooseclassroomswidget.hpp"
#include "lessontypestablemodel.hpp"

#include <cassert>


ChooseClassroomsDelegate::ChooseClassroomsDelegate(const QAbstractItemModel* model, QObject* parent)
    : QStyledItemDelegate(parent)
    , model_(model)
{
}

QWidget *
ChooseClassroomsDelegate::createEditor(QWidget* parent, const QStyleOptionViewItem&, const QModelIndex&) const
{
    auto editor = new ChooseClassroomsWidget(model_, parent);
    editor->setAutoFillBackground(true);
    return editor;
}

void ChooseClassroomsDelegate::setEditorData(QWidget* editor, const QModelIndex& index) const
{
    auto chooseClassroomsWidget = dynamic_cast<ChooseClassroomsWidget*>(editor);
    assert(chooseClassroomsWidget != nullptr);
    chooseClassroomsWidget->setClassrooms(index.data(Qt::ItemDataRole::UserRole).value<ClassroomsSet>());
}

void ChooseClassroomsDelegate::setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const
{
    auto chooseClassroomsWidget = dynamic_cast<ChooseClassroomsWidget*>(editor);
    assert(chooseClassroomsWidget != nullptr);
    model->setData(index, QVariant::fromValue(chooseClassroomsWidget->classrooms()));
}

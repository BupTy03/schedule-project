#include "chooseclassroomsdelegate.hpp"
#include "chooseitemswidget.hpp"
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
    auto editor = new ChooseItemsWidget(model_, tr("Выбор аудиторий"), tr("Выберите аудитории"), parent);
    editor->setAutoFillBackground(true);
    return editor;
}

void ChooseClassroomsDelegate::setEditorData(QWidget* editor, const QModelIndex& index) const
{
    auto chooseClassroomsWidget = dynamic_cast<ChooseItemsWidget*>(editor);
    assert(chooseClassroomsWidget != nullptr);
    chooseClassroomsWidget->setItems(index.data(Qt::ItemDataRole::UserRole).value<StringsSet>());
}

void ChooseClassroomsDelegate::setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const
{
    auto chooseClassroomsWidget = dynamic_cast<ChooseItemsWidget*>(editor);
    assert(chooseClassroomsWidget != nullptr);
    model->setData(index, QVariant::fromValue(chooseClassroomsWidget->items()));
}

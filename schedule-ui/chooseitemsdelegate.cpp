#include "chooseitemsdelegate.hpp"
#include "chooseitemswidget.hpp"
#include "lessontypestablemodel.hpp"

#include <cassert>


ChooseItemsDelegate::ChooseItemsDelegate(const QAbstractItemModel* model,
                                         QString dialogTitle,
                                         QString groupBoxWidgetTitle,
                                         QObject* parent)
    : QStyledItemDelegate(parent)
    , model_(model)
    , dialogTitle_(std::move(dialogTitle))
    , groupBoxWidgetTitle_(std::move(groupBoxWidgetTitle))
{
}

QWidget *
ChooseItemsDelegate::createEditor(QWidget* parent, const QStyleOptionViewItem&, const QModelIndex&) const
{
    auto editor = new ChooseItemsWidget(model_, dialogTitle_, groupBoxWidgetTitle_, parent);
    editor->setAutoFillBackground(true);
    return editor;
}

void ChooseItemsDelegate::setEditorData(QWidget* editor, const QModelIndex& index) const
{
    auto chooseItemsWidget = dynamic_cast<ChooseItemsWidget*>(editor);
    assert(chooseItemsWidget != nullptr);
    chooseItemsWidget->setItems(index.data(Qt::ItemDataRole::UserRole).value<StringsSet>());
}

void ChooseItemsDelegate::setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const
{
    auto chooseItemsWidget = dynamic_cast<ChooseItemsWidget*>(editor);
    assert(chooseItemsWidget != nullptr);
    model->setData(index, QVariant::fromValue(chooseItemsWidget->items()));
}

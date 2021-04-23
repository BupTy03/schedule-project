#include "qcomboboxdelegate.hpp"
#include <QtWidgets/QComboBox>


QComboboxDelegate::QComboboxDelegate(QStringList items, QObject* parent)
        : QStyledItemDelegate(parent)
        , items_(std::move(items))
{
}

QWidget* QComboboxDelegate::createEditor(QWidget* parent, const QStyleOptionViewItem&, const QModelIndex&) const
{
    auto editor = new QComboBox(parent);
    editor->addItems(items_);
    editor->setAutoFillBackground(true);
    return editor;
}

void QComboboxDelegate::setEditorData(QWidget* editor, const QModelIndex& index) const
{
    auto comboBox = qobject_cast<QComboBox*>(editor);
    assert(comboBox != nullptr);
    comboBox->setCurrentText(index.data().toString());
}

void QComboboxDelegate::setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const
{
    auto comboBox = qobject_cast<QComboBox*>(editor);
    assert(comboBox != nullptr);
    model->setData(index, comboBox->currentText());
}


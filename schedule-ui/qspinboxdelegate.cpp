#include "qspinboxdelegate.hpp"

#include <QtWidgets/QSpinBox>


QSpinBoxDelegate::QSpinBoxDelegate(int minVal, int maxVal, QObject* parent)
    : QStyledItemDelegate(parent)
    , minValue_(minVal)
    , maxValue_(maxVal)
{
}

QWidget* QSpinBoxDelegate::createEditor(QWidget* parent, const QStyleOptionViewItem&, const QModelIndex&) const
{
    auto editor = new QSpinBox(parent);
    editor->setRange(minValue_, maxValue_);
    editor->setAutoFillBackground(true);
    return editor;
}

void QSpinBoxDelegate::setEditorData(QWidget* editor, const QModelIndex& index) const
{
    auto spinBox = static_cast<QSpinBox*>(editor);
    spinBox->setValue(index.data().toInt());
}

void QSpinBoxDelegate::setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const
{
    auto spinBox = static_cast<QSpinBox*>(editor);
    model->setData(index, spinBox->value());
}

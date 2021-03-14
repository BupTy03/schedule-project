#include "chooseweekdaydelegate.hpp"
#include "weekdayeditor.hpp"


ChooseWeekDayDelegate::ChooseWeekDayDelegate(QObject* parent)
    : QStyledItemDelegate(parent)
{
}

QWidget* ChooseWeekDayDelegate::createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    (void)option;
    (void)index;

    return new WeekDayEditor(parent);
}

void ChooseWeekDayDelegate::setEditorData(QWidget* editor, const QModelIndex& index) const
{
    auto weekDayEditor = static_cast<WeekDayEditor*>(editor);
    weekDayEditor->setWeekDays(index.data(Qt::ItemDataRole::UserRole).value<WeekDaysType>());
}

void ChooseWeekDayDelegate::setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const
{
    auto weekDayEditor = static_cast<WeekDayEditor*>(editor);
    model->setData(index, QVariant::fromValue(weekDayEditor->weekDays()));
}

void ChooseWeekDayDelegate::updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    editor->setGeometry(QRect(option.rect.topLeft(), QSize(65, 130)));
}

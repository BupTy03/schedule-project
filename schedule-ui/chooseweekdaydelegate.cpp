#include "chooseweekdaydelegate.hpp"
#include "weekdayeditor.hpp"


static constexpr auto WEEK_DAYS_EDITOR_WIDTH = 65;
static constexpr auto WEEK_DAYS_EDITOR_HEIGHT = 130;


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
    auto topLeft = option.rect.topLeft();
    topLeft.ry() -= WEEK_DAYS_EDITOR_HEIGHT / 8;
    editor->setGeometry(QRect(topLeft, QSize(WEEK_DAYS_EDITOR_WIDTH, WEEK_DAYS_EDITOR_HEIGHT)));
}

#include "weekdayeditor.hpp"

#include <QStandardItemModel>
#include <QFocusEvent>
#include <QString>

#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QListView>

#include <array>


WeekDayEditor::WeekDayEditor(QWidget* parent)
    : QWidget(parent)
{
    static const std::array<QString, 6> daysNames = {
        QObject::tr("ПН"),
        QObject::tr("ВТ"),
        QObject::tr("СР"),
        QObject::tr("ЧТ"),
        QObject::tr("ПТ"),
        QObject::tr("СБ")
    };

    for (std::size_t d = 0; d < daysNames.size(); ++d)
    {
        auto item = new QStandardItem(daysNames.at(d));
        item->setCheckable(true);
        model_.appendRow(item);
    }

    auto layout = new QVBoxLayout;
    setLayout(layout);

    auto view = new QListView(this);
    layout->addWidget(view);
    view->setModel(&model_);
}

void WeekDayEditor::setWeekDays(const WeekDaysType& days)
{
    for (int d = 0; d < model_.rowCount(); ++d)
    {
        auto item = model_.item(d, 0);
        item->setCheckState(days.at(static_cast<std::size_t>(d)) ? Qt::CheckState::Checked : Qt::CheckState::Unchecked);
    }
}

WeekDaysType WeekDayEditor::weekDays() const
{
    WeekDaysType result = { false };
    for (int d = 0; d < model_.rowCount(); ++d)
    {
        auto item = model_.item(d, 0);
        result.at(static_cast<std::size_t>(d)) = item->checkState() == Qt::CheckState::Checked;
    }

    return result;
}

void WeekDayEditor::focusOutEvent(QFocusEvent* event)
{
    auto g = geometry();
    const auto p = mapToParent(mapFromGlobal(QCursor::pos()));
    if(!geometry().contains(p))
    {
        hide();
        event->accept();
    }

    QWidget::focusOutEvent(event);
}

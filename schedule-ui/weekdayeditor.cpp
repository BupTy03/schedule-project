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

void WeekDayEditor::setWeekDays(const WeekDays& days)
{
    for (int d = 0; d < model_.rowCount(); ++d)
    {
        auto item = model_.item(d, 0);
        item->setCheckState(days.contains(static_cast<WeekDay>(d)) ? Qt::CheckState::Checked : Qt::CheckState::Unchecked);
    }
}

WeekDays WeekDayEditor::weekDays() const
{
    WeekDays result;
    for (int d = 0; d < model_.rowCount(); ++d)
    {
        auto item = model_.item(d, 0);
        if(item->checkState() == Qt::CheckState::Checked)
            result.insert(static_cast<WeekDay>(d));
        else
            result.erase(static_cast<WeekDay>(d));
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

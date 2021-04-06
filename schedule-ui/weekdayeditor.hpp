#pragma once
#include "lessontypestablemodel.hpp"

#include <QWidget>
#include <QStandardItemModel>


class WeekDayEditor : public QWidget
{
    Q_OBJECT

public:
    explicit WeekDayEditor(QWidget* parent);

    void setWeekDays(const WeekDays& days);
    WeekDays weekDays() const;

    // QWidget interface
protected:
    void focusOutEvent(QFocusEvent* event) override;

private:
    QStandardItemModel model_;
};


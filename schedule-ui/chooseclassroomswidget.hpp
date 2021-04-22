#pragma once
#include "ScheduleCommon.hpp"

#include <QWidget>


class QLabel;
class QPushButton;
class QAbstractItemModel;

class ChooseItemsDialog;


class ChooseClassroomsWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ChooseClassroomsWidget(const QAbstractItemModel* model, QWidget* parent = nullptr);
    void setClassrooms(const SortedSet<QString>& classrooms);
    [[nodiscard]] SortedSet<QString> classrooms() const;

private slots:
    void onChooseButtonClicked();

private:
    QLabel* classroomsLabel_;
    QPushButton* chooseClassroomButton_;
    ChooseItemsDialog* chooseDialog_;
    SortedSet<QString> classrooms_;
};

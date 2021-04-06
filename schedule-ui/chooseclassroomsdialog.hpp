#pragma once
#include "ScheduleCommon.hpp"
#include <QString>
#include <QDialog>
#include <QStringListModel>


class ChooseItemsSetWidget;

class ChooseClassroomsDialog : public QDialog
{
    Q_OBJECT
public:
    explicit ChooseClassroomsDialog(const QAbstractItemModel* model, QWidget* parent = nullptr);
    [[nodiscard]] SortedSet<QString> classrooms() const;

private:
    ChooseItemsSetWidget* chooseWidget_;
};

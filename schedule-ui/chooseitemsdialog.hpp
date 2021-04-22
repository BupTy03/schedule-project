#pragma once
#include "ScheduleCommon.hpp"
#include <QString>
#include <QDialog>
#include <QStringListModel>


class ChooseItemsSetWidget;

class ChooseItemsDialog : public QDialog
{
    Q_OBJECT
public:
    explicit ChooseItemsDialog(const QAbstractItemModel* model,
                               const QString& title,
                               QWidget* parent = nullptr);
    [[nodiscard]] SortedSet<QString> items() const;

private:
    ChooseItemsSetWidget* chooseWidget_;
};

#pragma once

#include <QString>
#include <QDialog>
#include <QStringListModel>

#include <set>


class ChooseItemsSetWidget;

class ChooseClassroomsDialog : public QDialog
{
    Q_OBJECT
public:
    explicit ChooseClassroomsDialog(const QAbstractItemModel* model, QWidget* parent = nullptr);
    [[nodiscard]] std::set<QString> classrooms() const;

private:
    ChooseItemsSetWidget* chooseWidget_;
};

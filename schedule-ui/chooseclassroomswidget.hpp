#pragma once

#include <QWidget>
#include <set>


class QLabel;
class QPushButton;
class QAbstractItemModel;

class ChooseClassroomsDialog;


class ChooseClassroomsWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ChooseClassroomsWidget(const QAbstractItemModel* model, QWidget* parent = nullptr);
    void setClassrooms(const std::set<QString>& classrooms);
    [[nodiscard]] std::set<QString> classrooms() const;

private slots:
    void onChooseButtonClicked();

private:
    QLabel* classroomsLabel_;
    QPushButton* chooseClassroomButton_;
    ChooseClassroomsDialog* chooseDialog_;
    std::set<QString> classrooms_;
};

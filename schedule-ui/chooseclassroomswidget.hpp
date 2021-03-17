#pragma once

#include <QWidget>
#include <set>


class QLabel;
class QPushButton;

class ChooseClassroomWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ChooseClassroomWidget(QWidget* parent = nullptr);

    void setClassrooms(const std::set<QString>& classrooms);
    [[nodiscard]] std::set<QString> classrooms() const;

private:
    QLabel* classroomsLabel_;
    QPushButton* chooseClassroomButton_;
    std::set<QString> classrooms_;
};

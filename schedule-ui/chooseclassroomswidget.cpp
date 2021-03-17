#include "chooseclassroomswidget.hpp"
#include "utils.hpp"

#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QHBoxLayout>


ChooseClassroomWidget::ChooseClassroomWidget(QWidget* parent)
    : QWidget(parent)
    , classroomsLabel_(new QLabel(this))
    , chooseClassroomButton_(new QPushButton("...", this))
    , classrooms_()
{
    classroomsLabel_->setSizePolicy(QSizePolicy::Policy::MinimumExpanding, QSizePolicy::Policy::Fixed);
    chooseClassroomButton_->setSizePolicy(QSizePolicy::Policy::Fixed, QSizePolicy::Policy::Fixed);
    chooseClassroomButton_->setFixedWidth(30);

    auto layer = new QHBoxLayout(this);
    layer->addWidget(classroomsLabel_);
    layer->addWidget(chooseClassroomButton_);
    setLayout(layer);
}

void ChooseClassroomWidget::setClassrooms(const std::set<QString>& classrooms)
{
    classrooms_ = classrooms;
    classroomsLabel_->setText(Join(classrooms_, ", "));
}

std::set<QString> ChooseClassroomWidget::classrooms() const
{
    return classrooms_;
}

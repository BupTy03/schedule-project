#include "chooseclassroomswidget.hpp"
#include "chooseclassroomsdialog.hpp"
#include "utils.hpp"

#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QHBoxLayout>


ChooseClassroomsWidget::ChooseClassroomsWidget(QAbstractItemModel* model, QWidget* parent)
    : QWidget(parent)
    , classroomsLabel_(new QLabel(this))
    , chooseClassroomButton_(new QPushButton("...", this))
    , chooseDialog_(new ChooseClassroomsDialog(model, this))
    , classrooms_()
{
    classroomsLabel_->setSizePolicy(QSizePolicy::Policy::MinimumExpanding, QSizePolicy::Policy::Fixed);
    chooseClassroomButton_->setSizePolicy(QSizePolicy::Policy::Fixed, QSizePolicy::Policy::Fixed);
    chooseClassroomButton_->setFixedWidth(30);

    auto layer = new QHBoxLayout(this);
    layer->addWidget(classroomsLabel_);
    layer->addWidget(chooseClassroomButton_);
    setLayout(layer);

    connect(chooseClassroomButton_, &QPushButton::clicked, this, &ChooseClassroomsWidget::onChooseButtonClicked);
}

void ChooseClassroomsWidget::setClassrooms(const std::set<QString>& classrooms)
{
    classrooms_ = classrooms;
    classroomsLabel_->setText(Join(classrooms_, ", "));
}

std::set<QString> ChooseClassroomsWidget::classrooms() const
{
    return classrooms_;
}

void ChooseClassroomsWidget::onChooseButtonClicked()
{
    if(chooseDialog_->exec() != QDialog::DialogCode::Accepted)
        return;

    setClassrooms(chooseDialog_->classrooms());
}

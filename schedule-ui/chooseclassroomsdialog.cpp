#include "chooseclassroomsdialog.hpp"
#include "chooseitemssetwidget.hpp"

#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QDialogButtonBox>


ChooseClassroomsDialog::ChooseClassroomsDialog(const QAbstractItemModel* model, QWidget *parent)
    : QDialog(parent)
    , chooseWidget_(new ChooseItemsSetWidget(model, tr("Выберите аудитории"), this))
{
    setWindowTitle(tr("Выбор аудиторий"));
    setWindowFlags(Qt::Dialog | Qt::WindowTitleHint | Qt::WindowCloseButtonHint);

    auto layout = new QVBoxLayout(this);
    layout->addWidget(chooseWidget_);
    auto buttonBox = new QDialogButtonBox(QDialogButtonBox::StandardButton::Ok | QDialogButtonBox::Cancel, this);
    layout->addWidget(buttonBox);
    setLayout(layout);

    connect(buttonBox, &QDialogButtonBox::accepted, this, &ChooseClassroomsDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &ChooseClassroomsDialog::reject);
}

SortedSet<QString> ChooseClassroomsDialog::classrooms() const
{
    const auto chosenItems = chooseWidget_->chosenItemsList();
    return SortedSet<QString>(chosenItems.begin(), chosenItems.end());
}

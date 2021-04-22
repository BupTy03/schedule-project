#include "chooseitemsdialog.hpp"
#include "chooseitemssetwidget.hpp"

#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QDialogButtonBox>


ChooseItemsDialog::ChooseItemsDialog(const QAbstractItemModel* model,
                                     const QString& groupTitle,
                                     QWidget *parent)
    : QDialog(parent)
    , chooseWidget_(new ChooseItemsSetWidget(model, groupTitle, this))
{
    setWindowFlags(Qt::Dialog | Qt::WindowTitleHint | Qt::WindowCloseButtonHint);

    auto layout = new QVBoxLayout(this);
    layout->addWidget(chooseWidget_);

    auto buttonBox = new QDialogButtonBox(QDialogButtonBox::StandardButton::Ok | QDialogButtonBox::Cancel, this);
    layout->addWidget(buttonBox);
    setLayout(layout);

    connect(buttonBox, &QDialogButtonBox::accepted, this, &ChooseItemsDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &ChooseItemsDialog::reject);
}

SortedSet<QString> ChooseItemsDialog::items() const
{
    return chooseWidget_->chosenItemsList();
}

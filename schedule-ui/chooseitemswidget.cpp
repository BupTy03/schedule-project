#include "chooseitemswidget.hpp"
#include "chooseitemsdialog.hpp"
#include "utils.hpp"

#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QHBoxLayout>


ChooseItemsWidget::ChooseItemsWidget(const QAbstractItemModel* model,
                                     const QString& dialogTitle,
                                     const QString& groupTitle,
                                     QWidget* parent)
    : QWidget(parent)
    , itemsLabel_(new QLabel(this))
    , chooseItemButton_(new QPushButton("...", this))
    , chooseDialog_(new ChooseItemsDialog(model, groupTitle, this))
    , items_()
{
    chooseDialog_->setWindowTitle(dialogTitle);

    itemsLabel_->setSizePolicy(QSizePolicy::Policy::MinimumExpanding, QSizePolicy::Policy::Fixed);
    chooseItemButton_->setSizePolicy(QSizePolicy::Policy::Fixed, QSizePolicy::Policy::Fixed);
    chooseItemButton_->setFixedWidth(30);
    chooseItemButton_->setFixedHeight(20);

    auto layer = new QHBoxLayout(this);
    layer->addWidget(itemsLabel_);
    layer->addWidget(chooseItemButton_);
    setLayout(layer);

    connect(chooseItemButton_, &QPushButton::clicked, this, &ChooseItemsWidget::onChooseButtonClicked);
}

void ChooseItemsWidget::setItems(const SortedSet<QString>& items)
{
    items_ = items;
    itemsLabel_->setText(Join(items_, ", "));
}

SortedSet<QString> ChooseItemsWidget::items() const
{
    return items_;
}

void ChooseItemsWidget::onChooseButtonClicked()
{
    if(chooseDialog_->exec() != QDialog::DialogCode::Accepted)
        return;

    setItems(chooseDialog_->items());
}

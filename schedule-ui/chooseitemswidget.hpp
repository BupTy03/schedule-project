#pragma once
#include "ScheduleUtils.hpp"
#include <QWidget>


class QLabel;
class QPushButton;
class QAbstractItemModel;

class ChooseItemsDialog;


class ChooseItemsWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ChooseItemsWidget(const QAbstractItemModel* model,
                               const QString& dialogTitle,
                               const QString& groupTitle,
                               QWidget* parent = nullptr);
    void setItems(const SortedSet<QString>& items);
    SortedSet<QString> items() const;

private slots:
    void onChooseButtonClicked();

private:
    QLabel* itemsLabel_;
    QPushButton* chooseItemButton_;
    ChooseItemsDialog* chooseDialog_;
    SortedSet<QString> items_;
};

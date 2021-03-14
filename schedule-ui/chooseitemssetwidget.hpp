#pragma once

#include <QWidget>
#include <QAbstractItemModel>
#include <QStringListModel>


namespace Ui
{
class ChooseItemsSetWidget;
}

class ChooseItemsSetWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ChooseItemsSetWidget(const QAbstractItemModel* sourceModel,
                                  const QString& title,
                                  QWidget* parent = nullptr);
    ~ChooseItemsSetWidget() override;

    void setAvailableAndChosenTitles(const QString& availableTitle,
                                     const QString& chosenTitle);

    QStringList chosenItemsList() const;

    // QWidget interface
protected:
    void showEvent(QShowEvent* event) override;

private slots:
    void addItemButtonClicked();
    void removeItemButtonClicked();

private:
    Ui::ChooseItemsSetWidget* ui;
    const QAbstractItemModel* sourceModel_;
    QStringListModel availableItemsModel_;
    QStringListModel chosenItemsModel_;
};

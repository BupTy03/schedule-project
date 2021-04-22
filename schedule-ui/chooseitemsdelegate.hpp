#pragma once

#include <QStyledItemDelegate>

class QAbstractItemModel;

class ChooseItemsDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit ChooseItemsDelegate(const QAbstractItemModel* model,
                                 QString dialogTitle,
                                 QString groupBoxWidgetTitle,
                                 QObject* parent = nullptr);

    // QAbstractItemDelegate interface
public:
    QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem&, const QModelIndex&) const override;
    void setEditorData(QWidget* editor, const QModelIndex& index) const override;
    void setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const override;

private:
    const QAbstractItemModel* model_;
    QString dialogTitle_;
    QString groupBoxWidgetTitle_;
};

#pragma once

#include <QStyledItemDelegate>

class QAbstractItemModel;

class ChooseClassroomsDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit ChooseClassroomsDelegate(const QAbstractItemModel* model, QObject* parent = nullptr);

    // QAbstractItemDelegate interface
public:
    QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem&, const QModelIndex&) const override;
    void setEditorData(QWidget* editor, const QModelIndex& index) const override;
    void setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const override;

private:
    const QAbstractItemModel* model_;
};

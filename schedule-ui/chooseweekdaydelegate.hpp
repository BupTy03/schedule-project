#pragma once

#include <QStyledItemDelegate>


class ChooseWeekDayDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit ChooseWeekDayDelegate(QObject* parent = nullptr);

    // QAbstractItemDelegate interface
public:
    QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
    void setEditorData(QWidget* editor, const QModelIndex& index) const override;
    void setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const override;
    void updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
};

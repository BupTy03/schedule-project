#pragma once

#include <QStyledItemDelegate>


class QSpinBoxDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit QSpinBoxDelegate(int minVal, int maxVal, QObject* parent = nullptr);

    // QAbstractItemDelegate interface
public:
    QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem&, const QModelIndex&) const override;
    void setEditorData(QWidget* editor, const QModelIndex& index) const override;
    void setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const override;

private:
    int minValue_;
    int maxValue_;
};

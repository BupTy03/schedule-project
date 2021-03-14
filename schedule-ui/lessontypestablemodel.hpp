#pragma once
#include "scheduleimportexport.hpp"
#include <QAbstractTableModel>


class LessonTypesTableModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    explicit LessonTypesTableModel(QObject* parent = nullptr);

    const std::vector<LessonTypeItem>& lessons() const;

public:// QAbstractTableModel interface
    int rowCount(const QModelIndex&) const override;
    int columnCount(const QModelIndex&) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    QVariant data(const QModelIndex& index, int role) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role) override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;

private:
    std::vector<LessonTypeItem> lessons_;
};

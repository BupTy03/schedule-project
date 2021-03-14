#pragma once
#include "scheduleimportexport.hpp"
#include <QAbstractTableModel>


class DisciplinesModel : public QAbstractTableModel
{
public:
    explicit DisciplinesModel(QObject* parent = nullptr);

    void addDiscipline(const Discipline& discipline);
    void setDisciplines(const std::vector<Discipline>& disciplines);
    const std::vector<Discipline>& disciplines() const;

public:// QAbstractTableModel interface
    int rowCount(const QModelIndex& parent) const override;
    int columnCount(const QModelIndex& parent) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    QVariant data(const QModelIndex& index, int role) const override;
    bool removeRows(int row, int count, const QModelIndex& parent) override;

private:
    std::vector<Discipline> disciplines_;
};

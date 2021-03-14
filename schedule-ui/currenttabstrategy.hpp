#pragma once
#include <QIcon>


class QTableView;
class QAbstractItemView;
class AddDisciplineDialog;
class DisciplinesModel;


enum class CurrentTabType
{
    Disciplines,
    Groups,
    Professors,
    Classrooms
};


class CurrentTabStrategy
{
public:
    virtual ~CurrentTabStrategy();

    virtual QIcon addActionIcon() const;
    virtual QString addActionToolTip() const = 0;

    virtual QIcon removeActionIcon() const;
    virtual QString removeActionToolTip() const = 0;

    virtual void onAddItem() = 0;
    virtual void onRemoveItem() = 0;
};


class GeneralTabStrategy : public CurrentTabStrategy
{
public:
    explicit GeneralTabStrategy(QAbstractItemView* view,
                                 QString addActionToolTip,
                                 QString removeActionToolTip);

    QString addActionToolTip() const override;
    QString removeActionToolTip() const override;

    void onAddItem() override;
    void onRemoveItem() override;

private:
    QAbstractItemView* view_;
    QString addActionToolTip_;
    QString removeActionToolTip_;
};


class DisciplinesTabStrategy : public CurrentTabStrategy
{
public:
    explicit DisciplinesTabStrategy(AddDisciplineDialog* addDisciplineWidget,
                                    DisciplinesModel* disciplinesModel,
                                    QTableView* disciplinesView);

    QString addActionToolTip() const override;
    QString removeActionToolTip() const override;

    void onAddItem() override;
    void onRemoveItem() override;

private:
    AddDisciplineDialog* addDisciplineWidget_;
    DisciplinesModel* disciplinesModel_;
    QTableView* disciplinesView_;
};

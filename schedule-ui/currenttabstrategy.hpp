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
    Professors
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


class AbstractTabStrategy : public CurrentTabStrategy
{
public:
    explicit AbstractTabStrategy(QAbstractItemView* view);

    void onAddItem() override;
    void onRemoveItem() override;

private:
    QAbstractItemView* view_;
};


class GroupsTabStrategy : public AbstractTabStrategy
{
public:
    explicit GroupsTabStrategy(QAbstractItemView* view);

    QString addActionToolTip() const override;
    QString removeActionToolTip() const override;
};


class ProfessorsTabStrategy : public AbstractTabStrategy
{
public:
    explicit ProfessorsTabStrategy(QAbstractItemView* view);

    QString addActionToolTip() const override;
    QString removeActionToolTip() const override;
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

#pragma once
#include "scheduleimportexport.hpp"
#include <QDialog>
#include <QStringListModel>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QSpinBox>
#include <memory>


class ChooseItemsSetWidget;
class LessonTypesTableModel;

namespace Ui
{
class AddDisciplineDialog;
}

class AddDisciplineDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AddDisciplineDialog(QStringListModel* groupsListModel,
                                 QStringListModel* professorListModel,
                                 QStringListModel* classroomsListModel,
                                 QWidget* parent = nullptr);
    ~AddDisciplineDialog() override;

    [[nodiscard]] Discipline discipline() const;
    void keyPressEvent(QKeyEvent* evt) override;

private slots:
    void onOkButtonClicked();

private:
    Ui::AddDisciplineDialog* ui;
    QComboBox* professorChoose_;
    QComboBox* disciplineChoose_;
    std::unique_ptr<LessonTypesTableModel> lessonsModel_;
};

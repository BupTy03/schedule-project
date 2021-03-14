#pragma once
#include <QMainWindow>
#include <QStringListModel>

#include <memory>


QT_BEGIN_NAMESPACE
namespace Ui
{
class MainWindow;
}
QT_END_NAMESPACE

class QToolBar;

class AddDisciplineDialog;
class CurrentTabStrategy;
class ScheduleDataStorage;
class DisciplinesModel;


class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(std::unique_ptr<ScheduleDataStorage> scheduleData,
                        QWidget* parent = nullptr);
    ~MainWindow() override;

private slots:
    void onTabChanged(int current);

private:
    Ui::MainWindow* ui;
    QToolBar* toolBar_;
    QStringListModel groupsListModel_;
    QStringListModel professorsListModel_;
    AddDisciplineDialog* addDisciplineDialog_;
    std::unique_ptr<CurrentTabStrategy> tabStrategy_;
    std::unique_ptr<ScheduleDataStorage> scheduleData_;
    std::unique_ptr<DisciplinesModel> disciplinesModel_;
};

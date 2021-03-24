#pragma once
#include "schedulemodel.hpp"
#include <QDialog>


namespace Ui
{
class ShowScheduleDialog;
}

class ShowScheduleDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ShowScheduleDialog(QWidget* parent = nullptr);
    ~ShowScheduleDialog() override;

    void setViewMode(bool flag);
    void setSchedule(const std::vector<GroupSchedule>& evenSchedule,
                     const std::vector<GroupSchedule>& oddSchedule);

private:
    Ui::ShowScheduleDialog* ui;
    ScheduleModel evenScheduleModel_;
    ScheduleModel oddScheduleModel_;
};

void SetViewMode(ShowScheduleDialog& dialog);
void SetShowResultMode(ShowScheduleDialog& dialog);

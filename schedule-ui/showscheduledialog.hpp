#pragma once

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

private:
    Ui::ShowScheduleDialog* ui;
};

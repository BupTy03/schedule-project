#include "showscheduledialog.hpp"
#include "ui_showscheduledialog.h"
#include "schedulemodel.hpp"
#include "hierarchicalheaderview.hpp"

#include <QtWidgets/QPushButton>
#include <QtWidgets/QTableView>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QHeaderView>

#include <QStringListModel>
#include <QStandardItem>
#include <QStandardItemModel>

#include <array>


static std::unique_ptr<QStandardItemModel> BuildWeekDayHeaderModel()
{
    auto result = std::make_unique<QStandardItemModel>();

    const std::array<QString, 6> days = {
        "Понедельник",
        "Вторник",
        "Среда",
        "Четверг",
        "Пятница",
        "Суббота"
    };

    for (std::size_t i = 0; i < days.size(); ++i)
    {
        QStandardItem* rotatedTextCell = new QStandardItem(days.at(i));
        rotatedTextCell->setSizeHint(QSize(rotatedTextCell->sizeHint().width() + 30, rotatedTextCell->sizeHint().height()));
        rotatedTextCell->setData(1, Qt::UserRole);

        for (int i = 1; i <= MAX_LESSONS_PER_DAY_COUNT; ++i)
        {
            auto item = new QStandardItem(QString::number(i));
            item->setSizeHint(QSize(item->sizeHint().width() + 20, item->sizeHint().height()));
            rotatedTextCell->appendColumn({ item });
        }

        result->setItem(0, static_cast<int>(i), rotatedTextCell);
    }

    return result;
}


ShowScheduleDialog::ShowScheduleDialog(QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::ShowScheduleDialog)
    , evenScheduleModel_()
    , oddScheduleModel_()
{
    ui->setupUi(this);
    setWindowTitle(tr("Расписание"));
    setWindowFlags(Qt::Dialog | Qt::WindowTitleHint | Qt::WindowCloseButtonHint | Qt::WindowMinimizeButtonHint | Qt::WindowMaximizeButtonHint);

    ui->buttonBox->button(QDialogButtonBox::StandardButton::Save)->setText(tr("Сохранить"));
    ui->buttonBox->button(QDialogButtonBox::StandardButton::Close)->setText(tr("Закрыть"));

    ui->tabWidget->setTabText(0, tr("Числитель"));
    ui->tabWidget->setTabText(1, tr("Знаменатель"));

    auto evenView = new QTableView;
    evenView->setModel(&evenScheduleModel_);

    auto evenWdhw = new HierarchicalHeaderView(Qt::Orientation::Vertical, BuildWeekDayHeaderModel());
    evenView->setVerticalHeader(evenWdhw);

    auto evenLayout = new QGridLayout;
    evenLayout->addWidget(evenView);
    ui->evenTab->setLayout(evenLayout);


    auto oddView = new QTableView;
    oddView->setModel(&oddScheduleModel_);

    auto oddWdhw = new HierarchicalHeaderView(Qt::Orientation::Vertical, BuildWeekDayHeaderModel());
    oddView->setVerticalHeader(oddWdhw);

    auto oddLayout = new QGridLayout;
    oddLayout->addWidget(oddView);
    ui->oddTab->setLayout(oddLayout);
}

ShowScheduleDialog::~ShowScheduleDialog()
{
    delete ui;
}

void ShowScheduleDialog::setSchedule(const std::vector<GroupSchedule>& evenSchedule,
                                     const std::vector<GroupSchedule>& oddSchedule)
{
    evenScheduleModel_.setGroups(evenSchedule);
    oddScheduleModel_.setGroups(oddSchedule);
}

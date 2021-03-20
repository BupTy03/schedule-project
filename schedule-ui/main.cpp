#include "mainwindow.hpp"
#include "scheduleimportexport.hpp"

#include <QMessageBox>
#include <QApplication>


int main(int argc, char* argv[])
{
    QApplication a(argc, argv);
    auto scheduleDataStorage = std::make_unique<ScheduleDataJsonFile>("data.json");
    if(!scheduleDataStorage->IsValid())
    {
        QMessageBox::critical(nullptr, QObject::tr("Ошибка"), QObject::tr("Не удалось загрузить данные расписания."));
        return -1;
    }

    MainWindow w(std::move(scheduleDataStorage));
    w.setWindowIcon(QIcon(":/img/calendar.ico"));
    w.show();

    return a.exec();
}

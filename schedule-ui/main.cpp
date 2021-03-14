#include "mainwindow.hpp"
#include "scheduleimportexport.hpp"
#include "ScheduleCommon.hpp"

#include <QApplication>
#include <QDebug>


int main(int argc, char* argv[])
{
    QApplication a(argc, argv);
    qDebug() << static_cast<int>(ScheduleDayNumberToWeekDay(11));
    MainWindow w(std::make_unique<ScheduleDataJsonFile>("data.json"));
    w.show();
    return a.exec();
}

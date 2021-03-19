#include "mainwindow.hpp"
#include "scheduleimportexport.hpp"

#include <QApplication>


int main(int argc, char* argv[])
{
    QApplication a(argc, argv);
    MainWindow w(std::make_unique<ScheduleDataJsonFile>("data.json"));
    w.setWindowIcon(QIcon(":/img/calendar.ico"));
    w.show();

    return a.exec();
}

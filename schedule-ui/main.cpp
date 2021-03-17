#include "mainwindow.hpp"
#include "scheduleimportexport.hpp"
#include "chooseclassroomswidget.hpp"

#include <QApplication>


int main(int argc, char* argv[])
{
    QApplication a(argc, argv);
//    MainWindow w(std::make_unique<ScheduleDataJsonFile>("data.json"));
//    w.show();
    ChooseClassroomWidget w;
    w.setClassrooms(std::set<QString>{"1", "2", "3", "4"});
    w.show();
    return a.exec();
}

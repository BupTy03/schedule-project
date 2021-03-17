#include "mainwindow.hpp"
#include "scheduleimportexport.hpp"
#include "chooseclassroomswidget.hpp"
#include "chooseclassroomsdialog.hpp"

#include <QApplication>


int main(int argc, char* argv[])
{
    QApplication a(argc, argv);
//    MainWindow w(std::make_unique<ScheduleDataJsonFile>("data.json"));
//    w.show();

    QStringListModel model(QStringList() << "1" << "2" << "3" << "4");
    ChooseClassroomsWidget w(&model);
    w.show();

    return a.exec();
}

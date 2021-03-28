cmake_minimum_required(VERSION 3.5)

project(schedule-ui LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# QtCreator supports the following variables for Android, which are identical to qmake Android variables.
# Check http://doc.qt.io/qt-5/deployment-android.html for more information.
# They need to be set before the find_package(Qt5 ...) call.

#if(ANDROID)
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
#    if (ANDROID_ABI STREQUAL "armeabi-v7a")
#        set(ANDROID_EXTRA_LIBS
#            ${CMAKE_CURRENT_SOURCE_DIR}/path/to/libcrypto.so
#            ${CMAKE_CURRENT_SOURCE_DIR}/path/to/libssl.so)
#    endif()
#endif()

find_package(QT NAMES Qt6 Qt5 COMPONENTS Widgets REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Widgets REQUIRED)

set(PROJECT_SOURCES
        main.cpp
        mainwindow.cpp mainwindow.hpp mainwindow.ui
        showscheduledialog.cpp showscheduledialog.hpp showscheduledialog.ui
        schedulemodel.cpp schedulemodel.hpp
        hierarchicalheaderview.cpp hierarchicalheaderview.hpp
        adddisciplinedialog.cpp adddisciplinedialog.hpp adddisciplinedialog.ui
        qstringsetfilterproxymodel.cpp qstringsetfilterproxymodel.hpp
        chooseitemssetwidget.cpp chooseitemssetwidget.hpp chooseitemssetwidget.ui
        lessontypestablemodel.cpp lessontypestablemodel.hpp
        qspinboxdelegate.cpp qspinboxdelegate.hpp
        chooseweekdaydelegate.cpp chooseweekdaydelegate.hpp
        weekdayeditor.cpp weekdayeditor.hpp
        currenttabstrategy.cpp currenttabstrategy.hpp
        scheduleimportexport.cpp scheduleimportexport.hpp
        disciplinesmodel.cpp disciplinesmodel.hpp
        chooseclassroomswidget.cpp chooseclassroomswidget.hpp
        chooseclassroomsdialog.cpp chooseclassroomsdialog.hpp
        chooseclassroomsdelegate.cpp chooseclassroomsdelegate.hpp
        scheduleprocessor.cpp scheduleprocessor.hpp
        scheduleapplicationdatamodel.cpp scheduleapplicationdatamodel.hpp
        utils.cpp utils.hpp
        resources.qrc)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(schedule-ui
        ${PROJECT_SOURCES}
    )
else()
    if(ANDROID)
        add_library(schedule-ui SHARED
            ${PROJECT_SOURCES}
        )
    else()
        add_executable(schedule-ui
            ${PROJECT_SOURCES}
        )
    endif()
endif()

target_link_libraries(schedule-ui PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)
target_link_libraries(schedule-ui PUBLIC schedule-gen)

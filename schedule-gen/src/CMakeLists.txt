list(APPEND CMAKE_PREFIX_PATH "D:/Libs/ortools_VC2019_x64_Debug")

find_package(ortools CONFIG REQUIRED)

add_library(schedule-gen STATIC
        main.cpp
        ScheduleData.cpp
        ScheduleResult.cpp
        ScheduleCommon.cpp
        SATScheduleGenerator.cpp)

target_include_directories(schedule-gen PUBLIC ../include)
target_link_libraries(schedule-gen PRIVATE ortools::ortools)
target_compile_features(schedule-gen PUBLIC cxx_std_17)

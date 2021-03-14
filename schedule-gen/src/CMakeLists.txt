add_library(schedule-gen STATIC
        main.cpp
        ScheduleData.cpp
        ScheduleResult.cpp ScheduleGenerator.cpp ScheduleCommon.cpp)

target_include_directories(schedule-gen PUBLIC ../include)
target_compile_features(schedule-gen PUBLIC cxx_std_17)

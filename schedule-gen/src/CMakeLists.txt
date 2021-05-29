add_library(schedule-gen STATIC
        main.cpp
        ScheduleData.cpp
        ScheduleResult.cpp
        ScheduleCommon.cpp
        GAScheduleGenerator.cpp
        ScheduleUtils.cpp ScheduleGenerator.cpp)

target_include_directories(schedule-gen PUBLIC ../include)
target_compile_features(schedule-gen PUBLIC cxx_std_17)

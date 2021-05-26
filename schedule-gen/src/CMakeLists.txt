add_library(schedule-gen STATIC
        main.cpp
        ScheduleData.cpp
        ScheduleResult.cpp
        ScheduleCommon.cpp
        GAScheduleGenerator.cpp
        ScheduleUtils.cpp)

target_include_directories(schedule-gen PUBLIC ../include)
target_compile_features(schedule-gen PUBLIC cxx_std_17)

if(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
    list(APPEND CMAKE_PREFIX_PATH "D:/Libs/ortools_VC2019_x64_Debug")
    find_package(ortools CONFIG REQUIRED)
    target_link_libraries(schedule-gen PRIVATE ortools::ortools)
else()
    target_include_directories(schedule-gen PUBLIC "D:/Libs/or-tools_VisualStudio2019-64bit_v8.1.8487/include")
    target_link_directories(schedule-gen PUBLIC "D:/Libs/or-tools_VisualStudio2019-64bit_v8.1.8487/lib")
    target_link_libraries(schedule-gen PRIVATE ortools)
endif()

cmake_minimum_required(VERSION 3.17)
project(ScheduleProject)

set(CMAKE_CXX_STANDARD 20)

include(cmake/Conan.cmake)
run_conan()

# Automatically enable catch2 to generate ctest targets
if(CONAN_CATCH2_ROOT_DEBUG)
    include(${CONAN_CATCH2_ROOT_DEBUG}/lib/cmake/Catch2/Catch.cmake)
else()
    include(${CONAN_CATCH2_ROOT}/lib/cmake/Catch2/Catch.cmake)
endif()

# testing
add_library(catch_main STATIC catch_main.cpp)
target_link_libraries(catch_main PUBLIC CONAN_PKG::catch2)

add_subdirectory(schedule_gen)
add_subdirectory(schedule_web)

install(TARGETS schedule_web DESTINATION bin)

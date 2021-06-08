cmake_minimum_required(VERSION 3.17)
project(schedule-web)

include(cmake/Conan.cmake)
run_conan()

enable_testing()
add_subdirectory(test)
add_subdirectory(src)

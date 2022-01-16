cmake_minimum_required(VERSION 3.17)

if(DEFINED ENV{VCPKG_ROOT} AND NOT DEFINED CMAKE_TOOLCHAIN_FILE)
  set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake"
      CACHE STRING "")
endif()

project(schedule_project)
set(CMAKE_CXX_STANDARD 20)

find_package(Catch2 CONFIG REQUIRED)

list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/extras)
include(CTest)
include(Catch)

add_library(catch_main STATIC catch_main.cpp)
target_link_libraries(catch_main PUBLIC Catch2::Catch2)

add_subdirectory(schedule_gen)
add_subdirectory(schedule_web)

install(TARGETS schedule_web DESTINATION bin)

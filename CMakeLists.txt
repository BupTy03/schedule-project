cmake_minimum_required(VERSION 3.17)
project(ScheduleProject)

set(CMAKE_CXX_STANDARD 17)

set(BUILD_ZLIB ON)
set(BUILD_absl ON)
set(BUILD_Protobuf ON)
set(BUILD_SCIP ON)
set(BUILD_CoinUtils ON)
set(BUILD_Osi ON)
set(BUILD_Clp ON)
set(BUILD_Cgl ON)
set(BUILD_Cbc ON)

add_subdirectory(or-tools)
add_subdirectory(schedule-gen)
add_subdirectory(schedule-ui)

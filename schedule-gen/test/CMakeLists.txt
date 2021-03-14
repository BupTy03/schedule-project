# Automatically enable catch2 to generate ctest targets
if(CONAN_CATCH2_ROOT_DEBUG)
  include(${CONAN_CATCH2_ROOT_DEBUG}/lib/cmake/Catch2/Catch.cmake)
else()
  include(${CONAN_CATCH2_ROOT}/lib/cmake/Catch2/Catch.cmake)
endif()

add_library(catch_main STATIC catch_main.cpp ../include/ScheduleGenerator.h)
target_link_libraries(catch_main PUBLIC CONAN_PKG::catch2)

add_executable(Catch_test_schedule_gen test_schedule_gen.cpp ../src/ScheduleCommon.cpp)

target_include_directories(Catch_test_schedule_gen PRIVATE ../include)
target_link_libraries(Catch_test_schedule_gen PRIVATE catch_main schedule-gen)


catch_discover_tests(
  Catch_test_schedule_gen
  TEST_PREFIX
  "unittests."
  REPORTER
  xml
  OUTPUT_DIR
  .
  OUTPUT_PREFIX
  "unittests."
  OUTPUT_SUFFIX
  .xml)

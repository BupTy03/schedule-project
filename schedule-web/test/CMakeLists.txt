# Automatically enable catch2 to generate ctest targets
if(CONAN_CATCH2_ROOT_DEBUG)
  include(${CONAN_CATCH2_ROOT_DEBUG}/lib/cmake/Catch2/Catch.cmake)
else()
  include(${CONAN_CATCH2_ROOT}/lib/cmake/Catch2/Catch.cmake)
endif()

add_library(catch_main2 STATIC catch_main.cpp)
target_link_libraries(catch_main2 PUBLIC CONAN_PKG::catch2)

add_executable(Catch_test_schedule_web test_schedule_web.cpp)
target_link_libraries(Catch_test_schedule_web PRIVATE catch_main libschedule-web)


catch_discover_tests(
  Catch_test_schedule_web
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

project(schedule_gen)

file(GLOB SRC_FILES "src/*.cpp")
add_executable(${PROJECT_NAME} ${SRC_FILES})
target_include_directories(${PROJECT_NAME} PUBLIC include)

add_library(lib_${PROJECT_NAME} STATIC ${SRC_FILES})
target_include_directories(lib_${PROJECT_NAME} PUBLIC include)

add_executable(Catch_test_ScheduleChromosomes "tests/test_ScheduleChromosomes.cpp")
target_link_libraries(Catch_test_ScheduleChromosomes PUBLIC catch_main lib_${PROJECT_NAME})

catch_discover_tests(
    Catch_test_ScheduleChromosomes
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

add_executable(Catch_test_ScheduleGen "tests/test_ScheduleGen.cpp")
target_link_libraries(Catch_test_ScheduleGen PUBLIC catch_main lib_${PROJECT_NAME})

catch_discover_tests(
    Catch_test_ScheduleGen
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

add_executable(Catch_test_ScheduleUtils "tests/test_ScheduleUtils.cpp")
target_link_libraries(Catch_test_ScheduleUtils PUBLIC catch_main lib_${PROJECT_NAME})

catch_discover_tests(
    Catch_test_ScheduleUtils
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

add_executable(Catch_test_ScheduleValidation "tests/test_ScheduleValidation.cpp")
target_link_libraries(Catch_test_ScheduleValidation PUBLIC catch_main lib_${PROJECT_NAME})

catch_discover_tests(
    Catch_test_ScheduleValidation
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

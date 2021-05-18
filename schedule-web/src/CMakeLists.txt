# Download automatically, you can also just copy the conan.cmake file
if(NOT EXISTS "${CMAKE_BINARY_DIR}/conan.cmake")
	message(STATUS "Downloading conan.cmake from https://github.com/conan-io/cmake-conan")
	file(DOWNLOAD "https://github.com/conan-io/cmake-conan/raw/v0.15/conan.cmake"
			"${CMAKE_BINARY_DIR}/conan.cmake")
endif()

include(${CMAKE_BINARY_DIR}/conan.cmake)
conan_cmake_run(
		REQUIRES Poco/1.10.1@pocoproject/stable
		nlohmann_json/3.9.1
		BUILD missing)
conan_define_targets()

set(SRC_FILES
		main.cpp
		ScheduleServer.cpp
		ScheduleRequestHandler.cpp)

add_library(libschedule-web STATIC ${SRC_FILES})
target_include_directories(libschedule-web PUBLIC ../include)
target_compile_features(libschedule-web PUBLIC cxx_std_17)
target_link_libraries(libschedule-web PUBLIC schedule-gen)
target_link_libraries(libschedule-web PUBLIC CONAN_PKG::Poco CONAN_PKG::nlohmann_json)

add_executable(schedule-web ${SRC_FILES})
target_include_directories(schedule-web PUBLIC ../include)
target_compile_features(schedule-web PUBLIC cxx_std_17)
target_link_libraries(schedule-web PUBLIC schedule-gen)
target_link_libraries(schedule-web PRIVATE CONAN_PKG::Poco CONAN_PKG::nlohmann_json)

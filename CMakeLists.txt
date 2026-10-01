cmake_minimum_required(VERSION 4.0.0)

project(frame-lib VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(lib/spdlog)
add_subdirectory(lib/Catch2)

add_library(frame-lib
	src/frame-lib.cpp
)

target_include_directories(frame-lib
	PUBLIC
		include
)

target_link_libraries(frame-lib
	PRIVATE
		spdlog::spdlog
)

enable_testing()

add_executable(frame-lib_tests
	tests/frame-lib_tests.cpp
)

target_link_libraries(frame-lib_tests
	PRIVATE
		frame-lib
		Catch2::Catch2WithMain
)

include(CTest)
include(Catch)
catch_discover_tests(frame-lib_tests)

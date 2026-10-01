cmake_minimum_required(VERSION 3.10.0)
project(frame-lib VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_C_STANDARD 23)
set(CMAKE_CXX_STANDARD 26)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_library(frame-lib
	src/frame-lib.cpp
)

target_include_directories(frame-lib
	PUBLIC
		$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
		$<INSTALL_INTERFACE:include>
)

find_package(fmt CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)

target_link_libraries(frame-lib
	PRIVATE
		spdlog::spdlog
		fmt::fmt
)

# --- Tests ---

include(CTest)

if(BUILD_TESTING)
	find_package(Catch2 3 REQUIRED)

	add_executable(frame-lib_tests
		tests/frame-lib_tests.cpp
	)

	target_link_libraries(frame-lib_tests
		PRIVATE
			frame-lib
			Catch2::Catch2WithMain
	)

	include(Catch)
	catch_discover_tests(frame-lib_tests)
endif()

enable_testing()

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)

cmake_minimum_required(VERSION 3.15.7)
project(whiskey C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

find_package(Vulkan REQUIRED)

if (NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE "Release")
endif() # CMAKE_BUILD_TYPE

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if (WIN32)
	add_compile_definitions(WHIS_PLATFORM_WIN32)
endif() # WIN32

if (NOT WHIS_PLATFORM)
	message(FATAL_ERROR "Error: Platform not specified.")
endif() # WHIS_PLATFORM

string(TOUPPER "${WHIS_PLATFORM}" WHIS_PLATFORM)

if (WHIS_PLATFORM STREQUAL "X11")
	add_compile_definitions(WHIS_PLATFORM_X11)
	find_package(PkgConfig REQUIRED)
	pkg_check_modules(XCB REQUIRED xcb)
elseif (WHIS_PLATFORM STREQUAL "WAYLAND")
	add_compile_definitions(WHIS_PLATFORM_WAYLAND)
endif() # WHIS_PLATFORM

if (CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
	set(WHISKEY_IS_TOP_LVL ON)
else()
	set(WHISKEY_IS_TOP_LVL OFF)
endif() # WHISKEY_IS_TOP_LVL

if (WHISKEY_IS_TOP_LVL)
	set(WHIS_DEFAULT_LIB_TYPE "EXEC")
else()
	set(WHIS_DEFAULT_LIB_TYPE "ARCH")
endif() # WHISKEY_IS_TOP_LVL

set(WHIS_LIB_TYPE "${WHIS_DEFAULT_LIB_TYPE}" CACHE STRING "Library type.")

if (WHIS_LIB_TYPE STREQUAL "STATIC")
	add_library(${PROJECT_NAME} STATIC)
	target_compile_definitions(${PROJECT_NAME} PUBLIC WHIS_LIB_STATIC)
elseif (WHIS_LIB_TYPE STREQUAL "EXEC")
	add_executable(${PROJECT_NAME}
		${CMAKE_CURRENT_SOURCE_DIR}/tests/test.c)
	target_compile_definitions(${PROJECT_NAME} PUBLIC WHIS_LIB_EXEC)
else ()
	add_library(${PROJECT_NAME} SHARED)
	target_compile_definitions(${PROJECT_NAME} PUBLIC WHIS_LIB_SHARED)
endif() # WHIS_LIB_TYPE

if (WHIS_INCLUDE_TESTS)
	target_include_directories(${PROJECT_NAME} PRIVATE
		${CMAKE_CURRENT_SOURCE_DIR}/tests)
	target_sources(${PROJECT_NAME} PRIVATE
		${CMAKE_CURRENT_SOURCE_DIR}/tests/test.c)
endif() # WHIS_INCLUDE_TESTS

# Core files
target_include_directories(${PROJECT_NAME} PUBLIC
	${CMAKE_CURRENT_SOURCE_DIR}/include)

target_sources(${PROJECT_NAME} PRIVATE
	${CMAKE_CURRENT_SOURCE_DIR}/src/whis/linux.c)

if (WHIS_PLATFORM STREQUAL "X11")
	target_link_libraries(${PROJECT_NAME} PRIVATE ${XCB_LIBRARIES})

	target_include_directories(${PROJECT_NAME} PRIVATE ${XCB_INCLUDE_DIRS})

	target_sources(${PROJECT_NAME} PRIVATE
		${CMAKE_CURRENT_SOURCE_DIR}/src/whx/util.c
		${CMAKE_CURRENT_SOURCE_DIR}/src/whx/vulkan.c
		${CMAKE_CURRENT_SOURCE_DIR}/src/whx/window.c)
endif () # Platform X11

target_link_libraries(${PROJECT_NAME} PRIVATE Vulkan::Vulkan)

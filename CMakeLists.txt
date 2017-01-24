cmake_minimum_required(VERSION 3.0.2)
project (strinalyze)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmakemodules")

set(CXX_STANDARD c++11)
set(CMAKE_CXX_FLAGS_DEBUG   "${CMAKE_CXX_FLAGS_DEBUG}   -O0 -g3 -ggdb -fno-inline-functions")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -DNDEBUG -O3")
include(AppendCompilerFlags)
append_cxx_compiler_flags("-std=c++11 -ggdb3 -Wall -Wextra" "GCC" CMAKE_CXX_FLAGS)

# Usage: find_or_download_package(PACKAGE_NAME, FOUND_FLAG_PREFIX, TARGET_PREFIX)
include(ExternalProject)
include(find_or_download_package)
find_or_download_package(Glog GLOG glog)
find_or_download_package(GFlags GFLAGS gflags)

find_package( Threads )

if (CMAKE_BUILD_TYPE EQUAL "Release")
	add_definitions(-DGOOGLE_STRIP_LOG=1)
endif()


add_subdirectory(src)


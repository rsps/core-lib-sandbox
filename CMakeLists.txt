cmake_minimum_required (VERSION 3.16)

# https://cliutils.gitlab.io/modern-cmake/modern-cmake.pdf

#set (CMAKE_TOOLCHAIN_FILE ${CMAKE_CURRENT_SOURCE_DIR}/ToolchainFile.txt)

project (rsp-core-lib)

set (LIB_VERSION_MAJOR 0)
set (LIB_VERSION_MINOR 1)
set (LIB_VERSION_PATCH 0)

# configure a header file to pass some of the CMake settings
# to the source code
configure_file (
    "${PROJECT_SOURCE_DIR}/version-def.h.in"
    "${PROJECT_BINARY_DIR}/src/version-def.h"
)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug)
endif()

set (CMAKE_THREAD_PREFER_PTHREAD TRUE)
set (THREADS_PREFER_PTHREAD_FLAG TRUE)
find_package(Threads REQUIRED)
find_package(Freetype REQUIRED)

set (CMAKE_CXX_STANDARD 20)

# Must use GNUInstallDirs to install libraries into correct
# locations on all platforms.
include(GNUInstallDirs)

#message(STATUS "BinDir: ${CMAKE_CURRENT_BINARY_DIR}")

add_library("rsp-core-lib" STATIC)

target_link_libraries ("rsp-core-lib"
    Threads::Threads
    Freetype::Freetype
)

# Define headers for this library. PUBLIC headers are used for
# compiling the library, and will be added to consumers' build
# paths.
target_include_directories(rsp-core-lib
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        src
        ${CMAKE_CURRENT_BINARY_DIR}/src
        ${FREETYPE_INCLUDE_DIRS}
)

set (GCC_VALIDATION_FLAGS -pedantic -Wall -Wextra -Wold-style-cast
    -Wcast-align -Wcast-qual -Wctor-dtor-privacy -Wdisabled-optimization
    -Wformat=2 -Winit-self -Wlogical-op -Wmissing-declarations
    -Wmissing-include-dirs -Wnoexcept -Wold-style-cast -Woverloaded-virtual
    -Wredundant-decls -Wsign-conversion -Wsign-promo
    -Wstrict-null-sentinel -Wstrict-overflow=5 -Wswitch-default
    -Wundef -Wno-unused -Weffc++ -Werror -Wformat=2
    CACHE INTERNAL "Flags to let GCC validate code against common weaknesses"
)

target_compile_options(rsp-core-lib PRIVATE
    ${GCC_VALIDATION_FLAGS} 
    -Wshadow
    -DDEBUG_LOG -DUSE_FREETYPE -DFT_CONFIG_OPTION_ERROR_STRINGS -rdynamic
    )

if (${CMAKE_BUILD_TYPE} EQUAL Debug)
    target_compile_options(rsp-core-lib PRIVATE
        -O0 -g3
    )
else()
    target_compile_options(rsp-core-lib PRIVATE
        -D_FORTIFY_SOURCE=2 -O3
    )
endif()

add_subdirectory(src)

#--------------------------------------------------------
# Rules to make tests
#-----------------------

enable_testing()

# Include external projects Doctest from github.
include(FetchContent)   
find_package(Git REQUIRED)

FetchContent_Declare(
    DocTest
    GIT_REPOSITORY "https://github.com/onqtam/doctest"
    GIT_TAG "2.4.6"
)

FetchContent_MakeAvailable(DocTest)

add_executable("rsp-core-lib-test")
add_subdirectory(tests)
add_dependencies("rsp-core-lib-test" doctest)
target_include_directories("rsp-core-lib-test"
    PUBLIC
        ${doctest_SOURCE_DIR}/doctest
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    PRIVATE
        src
)
# Disable shadow warning to silence doctest...
target_compile_options(rsp-core-lib-test PRIVATE
    ${GCC_VALIDATION_FLAGS} 
    -DUSE_FREETYPE -DFT_CONFIG_OPTION_ERROR_STRINGS -rdynamic
    )


target_link_libraries ("rsp-core-lib-test"
    rsp-core-lib
)

add_test("tests" "rsp-core-lib-test")

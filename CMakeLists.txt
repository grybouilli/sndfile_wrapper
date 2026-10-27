cmake_minimum_required(VERSION 3.11...3.18)

project(sndfile_wrapper
  VERSION 1.0.0
  DESCRIPTION "A Modern C++ sndfile wrapper using CMake, doxygen, and Catch."
  HOMEPAGE_URL "https://github.com/grybouilli/sndfile_wrapper"
  LANGUAGES CXX
)
if(NOT DEFINED CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 20)
elseif(${CMAKE_CXX_STANDARD} LESS 20)
  message(FATAL_ERROR "Ort utils needs CMAKE_CXX_STANDARD to be at least 20. Current value: ${CMAKE_CXX_STANDARD}")
endif()

option(BUILD_DOCS "Build and Install Documents (Requires Doxygen)" OFF)
option(${PROJECT_NAME}_BUILD_TESTING "Build and run tests" OFF)

## Make sure that custom modules like FindXYZ are found
set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})

include(FetchContent)
if(${CMAKE_VERSION} VERSION_LESS 3.14)
  include(cmake/FetchContent_MakeAvailable.cmake)
endif()
include(GNUInstallDirs)
include(cmake/FetchLibsndfile.cmake)

## Define Target
## =============

add_library(${PROJECT_NAME} INTERFACE)
target_include_directories(${PROJECT_NAME}
  INTERFACE
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/include>
)
target_compile_features(${PROJECT_NAME} INTERFACE cxx_std_14)

if(PROJECT_NAME STREQUAL PROJECT_NAME AND BUILD_DOCS)
  find_package(Doxygen REQUIRED dot)
  if(Doxygen_FOUND)
    add_subdirectory(docs)
  else()
    message(STATUS "Doxygen not found, not building docs")
  endif()
endif()

## Testing
## =======



## flag RMOLIB_BUILD_TESTING for enable testing anyway
if((PROJECT_NAME STREQUAL PROJECT_NAME OR ${PROJECT_NAME}_BUILD_TESTING) AND BUILD_TESTING)
  if(PROJECT_NAME STREQUAL PROJECT_NAME)
    include(CTest)
  endif()
  add_subdirectory(test)
endif()

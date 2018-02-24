cmake_minimum_required(VERSION 3.2)
project(pms)

MESSAGE("CMake Version: ${CMAKE_VERSION}")
MESSAGE("Build Type: ${CMAKE_BUILD_TYPE}")

IF(DEFINED ${CMAKE_BUILD_TYPE} AND ${CMAKE_BUILD_TYPE} STREQUAL "Debug")
    add_definitions(-DPMS_DEBUG)
    #add_compile_options(-DPMS_DEBUG)
ELSE()
ENDIF()

SET(CMAKE_C_STANDARD 99)

ADD_SUBDIRECTORY(pms)
ADD_SUBDIRECTORY(util)
ADD_SUBDIRECTORY(mytest)
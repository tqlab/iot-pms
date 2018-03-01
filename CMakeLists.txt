cmake_minimum_required(VERSION 2.8)
project(pms)

MESSAGE("CMake Version: ${CMAKE_VERSION}")
MESSAGE("Build Type: ${CMAKE_BUILD_TYPE}")

IF(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_definitions(-DPMS_DEBUG)
    #add_compile_options(-DPMS_DEBUG)
    MESSAGE("-DPMS_DEBUG Definition added!")
ELSE()
ENDIF()

SET(CMAKE_C_STANDARD 99)

ADD_SUBDIRECTORY(pms)
ADD_SUBDIRECTORY(util)
ADD_SUBDIRECTORY(mytest)
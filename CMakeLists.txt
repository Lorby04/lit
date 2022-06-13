cmake_minimum_required(VERSION 2.8)

project(li_test)

SET(CMAKE_CXX_STANDARD 20)
SET (PROJECT_ROOT ${PROJECT_SOURCE_DIR})
SET (PROJECT_BINARY_DIR "${PROJECT_ROOT}/bin")
SET (PROJECT_SRC_DIR "${PROJECT_ROOT}/src")
aux_source_directory(${PROJECT_SRC_DIR} SRC_LIST)
include_directories("${PROJECT_ROOT}/include")

MESSAGE ("Source path: ${PROJECT_SRC_DIR}")
MESSAGE ("Source files: ${SRC_LIST}")
MESSAGE ("Binary path: ${PROJECT_BINARY_DIR}")

add_executable(${PROJECT_BINARY_DIR}/lit ${SRC_LIST})

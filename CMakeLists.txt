# Licensed to the Apache Software Foundation (ASF) under one
# or more contributor license agreements.  See the NOTICE file
# distributed with this work for additional information
# regarding copyright ownership.  The ASF licenses this file
# to you under the Apache License, Version 2.0 (the
# "License"); you may not use this file except in compliance
# with the License.  You may obtain a copy of the License at
#
#   http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing,
# software distributed under the License is distributed on an
# "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
# KIND, either express or implied.  See the License for the
# specific language governing permissions and limitations
# under the License.

cmake_minimum_required(VERSION 3.18)
project(lit C CXX)
SET(CMAKE_CXX_STANDARD 20)
option(LINK_SO "Whether examples are linked dynamically" OFF)

set(CMAKE_PREFIX_PATH ${OUTPUT_PATH})
include(FindThreads)

#Protobuf
include(FindProtobuf)
set(PROTO_DIR ${CMAKE_CURRENT_LIST_DIR}/resource/proto)
set(PROTO_SOURCE ${CMAKE_CURRENT_LIST_DIR}/resource/source)
file(GLOB PROTO_FILES "${PROTO_DIR}/*.proto")
#message("ProtoDir:${PROTO_DIR}, source:${PROTO_SOURCE},files:${PROTO_FILES}")
foreach(f ${PROTO_FILES})
    set(PROTOC_ARGS "${f} --proto_path=${PROTO_DIR} --cpp_out=${PROTO_SOURCE}")
#    message("File:${f}, args: ${PROTOC_ARGS}")
    execute_process(COMMAND protoc ${f} --proto_path=${PROTO_DIR} --cpp_out=${PROTO_SOURCE})
endforeach()
file(GLOB PROTO_SRCS "${PROTO_SOURCE}/*.pb.cc")
file(GLOB PROTO_HDRS "${PROTO_SOURCE}/*.pb.h" "${PROTO_SOURCE}/*.pb.hpp")
include_directories(${PRORO_SOURCE})

# Search for libthrift* by best effort. If it is not found and brpc is
# compiled with thrift protocol enabled, a link error would be reported.
find_library(THRIFT_LIB NAMES thrift)
if (NOT THRIFT_LIB)
    set(THRIFT_LIB "")
endif()
find_library(THRIFTNB_LIB NAMES thriftnb)
if (NOT THRIFTNB_LIB)
    set(THRIFTNB_LIB "")
endif()

#find_path(GPERFTOOLS_INCLUDE_DIR NAMES gperftools/heap-profiler.h)
#find_library(GPERFTOOLS_LIBRARIES NAMES tcmalloc_and_profiler)
#include_directories(${GPERFTOOLS_INCLUDE_DIR})

find_path(BRPC_INCLUDE_PATH NAMES brpc/server.h)
#message("BRPC include path: ${BRPC_INCLUDE_PATH}")
if(LINK_SO)
    find_library(BRPC_LIB NAMES brpc)
    #message("Link brpc as so:,${BRPC_LIB}")
else()
    find_library(BRPC_LIB NAMES libbrpc.a brpc)
    #message("Link brpc as static lib:,${BRPC_LIB}")
endif()

if((NOT BRPC_INCLUDE_PATH) OR (NOT BRPC_LIB))
    message(FATAL_ERROR "Fail to find brpc")
endif()
include_directories(${BRPC_INCLUDE_PATH})
#message("BRPC include path:, ${BRPC_INCLUDE_PATH}")

find_path(GFLAGS_INCLUDE_PATH gflags/gflags.h)
find_library(GFLAGS_LIBRARY NAMES gflags libgflags)
if((NOT GFLAGS_INCLUDE_PATH) OR (NOT GFLAGS_LIBRARY))
    message(FATAL_ERROR "Fail to find gflags")
endif()
include_directories(${GFLAGS_INCLUDE_PATH})

execute_process(
    COMMAND bash -c "grep \"namespace [_A-Za-z0-9]\\+ {\" ${GFLAGS_INCLUDE_PATH}/gflags/gflags_declare.h | head -1 | awk '{print $2}' | tr -d '\n'"
    OUTPUT_VARIABLE GFLAGS_NS
)
if(${GFLAGS_NS} STREQUAL "GFLAGS_NAMESPACE")
    execute_process(
        COMMAND bash -c "grep \"#define GFLAGS_NAMESPACE [_A-Za-z0-9]\\+\" ${GFLAGS_INCLUDE_PATH}/gflags/gflags_declare.h | head -1 | awk '{print $3}' | tr -d '\n'"
        OUTPUT_VARIABLE GFLAGS_NS
    )
endif()

if(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    include(CheckFunctionExists)
    CHECK_FUNCTION_EXISTS(clock_gettime HAVE_CLOCK_GETTIME)
    if(NOT HAVE_CLOCK_GETTIME)
        set(DEFINE_CLOCK_GETTIME "-DNO_CLOCK_GETTIME_IN_MAC")
    endif()
endif()

set(CMAKE_CPP_FLAGS "${DEFINE_CLOCK_GETTIME} -DGFLAGS_NS=${GFLAGS_NS}")
set(CMAKE_CXX_FLAGS "${CMAKE_CPP_FLAGS} -DNDEBUG -O2 -D__const__=__unused__ -pipe -W -Wall -Wno-unused-parameter") # -fPIC -fno-omit-frame-pointer")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DBRPC_ENABLE_CPU_PROFILER")

#if(CMAKE_VERSION VERSION_LESS "3.1.3")
#    if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
#        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
#    endif()
#    if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
#        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
#    endif()
#else()
#    set(CMAKE_CXX_STANDARD 11)
#    set(CMAKE_CXX_STANDARD_REQUIRED ON)
#endif()

find_path(LEVELDB_INCLUDE_PATH NAMES leveldb/db.h)
find_library(LEVELDB_LIB NAMES leveldb)
if ((NOT LEVELDB_INCLUDE_PATH) OR (NOT LEVELDB_LIB))
    message(FATAL_ERROR "Fail to find leveldb")
endif()
include_directories(${LEVELDB_INCLUDE_PATH})

if(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    set(OPENSSL_ROOT_DIR
        "/usr/local/opt/openssl"    # Homebrew installed OpenSSL
        )
endif()

find_package(OpenSSL)
include_directories(${OPENSSL_INCLUDE_DIR})


set(DYNAMIC_LIB
    ${CMAKE_THREAD_LIBS_INIT}
    ${GFLAGS_LIBRARY}
    ${PROTOBUF_LIBRARIES}
    ${LEVELDB_LIB}
    ${OPENSSL_CRYPTO_LIBRARY}
    ${OPENSSL_SSL_LIBRARY}
    ${THRIFT_LIB}
    ${THRIFTNB_LIB}
    dl
    )

if(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    set(DYNAMIC_LIB ${DYNAMIC_LIB}
        pthread
        "-framework CoreFoundation"
        "-framework CoreGraphics"
        "-framework CoreData"
        "-framework CoreText"
        "-framework Security"
        "-framework Foundation"
        "-Wl,-U,_MallocExtension_ReleaseFreeMemory"
#        "-Wl,-U,_ProfilerStart"
#        "-Wl,-U,_ProfilerStop"
        )
endif()

add_subdirectory(server)
add_subdirectory(client)

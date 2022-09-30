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

cmake_minimum_required(VERSION 3.10)
project(li_server C CXX)

option(LINK_SO "Whether examples are linked dynamically" OFF)
SET(CMAKE_CXX_STANDARD 20)
execute_process(
    COMMAND bash -c "find ${PROJECT_SOURCE_DIR}/../.. -type d -regex \".*output/include$\" | head -n1 | xargs dirname | tr -d '\n'"
    OUTPUT_VARIABLE OUTPUT_PATH
)

set(CMAKE_PREFIX_PATH ${OUTPUT_PATH})
SET (PROJECT_ROOT ${PROJECT_SOURCE_DIR})
SET (PROJECT_BINARY_DIR "${PROJECT_ROOT}/bin")
SET (PROJECT_SRC_DIR "${PROJECT_ROOT}/src")
aux_source_directory(${PROJECT_SRC_DIR} SRC_LIST)
include_directories("${PROJECT_ROOT}/include")

#MESSAGE ("Server Source path: ${PROJECT_SRC_DIR}")
#MESSAGE ("Server Source files: ${SRC_LIST}")
#MESSAGE (Server "Binary path: ${PROJECT_BINARY_DIR}")
#MESSAGE (Server "Binary path: ${PROJECT_BINARY_DIR}")

set(EXEC liserver)
add_executable(${EXEC} ${PROTO_SRCS} ${SRC_LIST})
set_target_properties(${EXEC} PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_LIST_DIR}/bin")
target_include_directories(${EXEC} PUBLIC ${PROTO_SOURCE})
target_link_libraries(${EXEC} ${BRPC_LIB} ${DYNAMIC_LIB}  ${GPERFTOOLS_LIBRARIES})

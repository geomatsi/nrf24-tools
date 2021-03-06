# - Find protobuf-c
# Find protobuf c implementation libraries, includes and the protoc-c compiler
# FindProtobufC was loosely based on FindProtobuf that is shipped with cmake
#
# Module defines:
#   PROTOBUFC_FOUND        - library, includes and compiler where found
#   PROTOBUFC_INCLUDE_DIRS - include directories
#   PROTOBUFC_LIBRARIES    - protobuf-c libraries
#   PROTOBUFC_EXECUTEABLE  - protobuf-c compiler
#
# Environment variables:
#   PROTOBUFC_ROOTDIR      - optional - rootdir of protobuf-c installation
#
# Cache entries:
#   PROTOBUFC_LIBRARY      - detected protobuf-c library
#   PROTOBUF_INCLUDE_DIR   - detected protobuf-c include dir(s)
#   PROTOBUF_COMPILER      - detected protobuf-c compiler
#
####
#
#  ====================================================================
#  Example:
#
#   find_package(ProtobufC REQUIRED)
#   include_directories(${PROTOBUFC_INCLUDE_DIRS})
#   include_directories(${CMAKE_CURRENT_BINARY_DIR})
#   PROTOBUFC_GENERATE_C(PROTO_SOURCES PROTO_HEADERS protobuf.dir protobuf.proto)
#   add_executable(bar bar.c ${PROTO_SRCS} ${PROTO_HDRS})
#   target_link_libraries(bar ${PROTOBUF_LIBRARIES})
#
# NOTE: You may need to link against pthreads, depending
#       on the platform.
#
#  ====================================================================

#=============================================================================
# Copyright 2013 Thinstuff Technologies GmbH
# Copyright 2013 Bernhard Miklautz <bmiklautz@thinstuff.at>
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#=============================================================================

set(PROTOBUFC_SOURCE_EXTENSION "pb-c.c")
set(PROTOBUFC_HEADER_EXTENSION "pb-c.h")

set(PROTOBUFC_COMMON_HEADER  "msg.proto.h")

function(PROTOBUFC_GENERATE_C SOURCES HEADERS PROTO_DIR)
	if(NOT ARGN)
		message(SEND_ERROR "Error: PROTOBUFC_GENERATE_C() called without any proto files")
		return()
	endif(NOT ARGN)

	set(OUTPUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/${PROTO_DIR}")

	foreach(PROTO_NAME ${ARGN})
		set(FIL ${PROTO_DIR}/${PROTO_NAME})

		get_filename_component(ABS_FIL ${FIL} ABSOLUTE)
		get_filename_component(FIL_WE ${FIL} NAME_WE)
		get_filename_component(FIL_PATH ${ABS_FIL} PATH)

		list(APPEND ${SOURCES} "${OUTPUT_DIR}/${FIL_WE}.${PROTOBUFC_SOURCE_EXTENSION}")
		list(APPEND ${HEADERS} "${OUTPUT_DIR}/${FIL_WE}.${PROTOBUFC_HEADER_EXTENSION}")
		add_custom_command(
			OUTPUT "${OUTPUT_DIR}/${FIL_WE}.${PROTOBUFC_SOURCE_EXTENSION}"
			"${OUTPUT_DIR}/${FIL_WE}.${PROTOBUFC_HEADER_EXTENSION}"
			COMMAND  ${PROTOBUFC_COMPILER}
			ARGS --c_out ${CMAKE_CURRENT_BINARY_DIR} -I ${CMAKE_CURRENT_SOURCE_DIR} -I ${FIL_PATH} ${ABS_FIL}
			DEPENDS ${ABS_FIL}
			COMMENT "Running protobuf-c compiler on ${FIL}"
			VERBATIM )

		file(APPEND "${OUTPUT_DIR}/${PROTOBUFC_COMMON_HEADER}" "#include \"${FIL_WE}.${PROTOBUFC_HEADER_EXTENSION}\"\n")
	endforeach()

	set_source_files_properties(${${SOURCES}} ${${HEADERS}} PROPERTIES GENERATED TRUE)
	set(${SOURCES} ${${SOURCES}} PARENT_SCOPE)
	set(${HEADERS} ${${HEADERS}} PARENT_SCOPE)
endfunction()

find_library(PROTOBUFC_LIBRARY
		NAMES "protobuf-c"
		PATHS "/usr" "/usr/local" "/opt" ENV PROTOBUFC_ROOTDIR
		PATH_SUFFIXES "lib")
mark_as_advanced(PROTOBUFC_LIBRARY)

find_path(PROTOBUFC_INCLUDE_DIR
		NAMES "google/protobuf-c/protobuf-c.h"
		PATHS "/usr" "/usr/local" "/opt" ENV PROTOBUFC_ROOTDIR
		PATH_SUFFIXES "include")
mark_as_advanced(PROTOBUFC_INCLUDE_DIR)

find_program(PROTOBUFC_COMPILER
		NAMES "protoc-c"
		PATHS "/usr" "/usr/local" "/opt" ENV PROTOBUFC_ROOTDIR
		PATH_SUFFIXES "bin")
mark_as_advanced(PROTOBUFC_COMPILER)

include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(
	ProtobufC
	DEFAULT_MSG PROTOBUFC_LIBRARY
	PROTOBUFC_COMPILER
	PROTOBUFC_INCLUDE_DIR )

if (PROTOBUFC_FOUND)
	set(PROTOBUFC_LIBRARIES ${PROTOBUFC_LIBRARY})
	set(PROTOBUFC_INCLUDE_DIRS ${PROTOBUFC_INCLUDE_DIR})
	set(PROTOBUFC_EXECUTEABLE ${PROTOBUFC_COMPILER})
endif(PROTOBUFC_FOUND)

# SPDX-License-Identifier: GPL-3.0-only

cmake_minimum_required(VERSION 3.10)

set(SOURCE_FILES 
	ebneuro.c
	network.c
	)
set(HEADER_FILES
	packets.h
	ebneuro.h
	)

add_library(ebneuro STATIC ${HEADER_FILES} ${SOURCE_FILES})
set_property(TARGET ebneuro PROPERTY POSITION_INDEPENDENT_CODE ON)

include_directories(../system)
include_directories(../../include)
target_link_libraries(ebneuro system)

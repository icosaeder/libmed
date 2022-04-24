# SPDX-License-Identifier: GPL-3.0-only

cmake_minimum_required(VERSION 3.10)

set(SOURCE_FILES 
	ebneuro_test.c
	)

add_executable(ebneuro_test ${SOURCE_FILES})
include_directories(../include)
target_link_libraries(ebneuro_test ebneuro)



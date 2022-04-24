# SPDX-License-Identifier: GPL-3.0-only

cmake_minimum_required(VERSION 3.10)

set(SOURCE_FILES 
	linux.c
	)
set(HEADER_FILES
	system.h
	endiannes.h
	helpers.h
	)

# TODO: detect the platform
add_library(system STATIC ${HEADER_FILES} ${SOURCE_FILES})


# SPDX-License-Identifier: GPL-3.0-only

cmake_minimum_required(VERSION 3.14)

project(
	libmed
	DESCRIPTION "A library for medical data acquisition"
	LANGUAGES C
)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

add_subdirectory(src)
add_subdirectory(apps)
add_subdirectory(examples)
add_subdirectory(python)

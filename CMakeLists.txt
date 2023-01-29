# SPDX-License-Identifier: GPL-3.0-only

cmake_minimum_required(VERSION 3.10)

project(
	libmed
	DESCRIPTION "A library for medical data acquisition"
	LANGUAGES C
)

add_subdirectory(src)
add_subdirectory(apps)
add_subdirectory(examples)

# SPDX-License-Identifier: GPL-3.0-only

add_library(system STATIC
	linux.c
	./include/system/system.h
	./include/system/endiannes.h
	./include/system/helpers.h
	)

target_include_directories(system PUBLIC include)


# SPDX-License-Identifier: GPL-3.0-only

add_library(openbci STATIC
	openbci.c
)

target_link_libraries(openbci PRIVATE med)

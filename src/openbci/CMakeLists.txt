# SPDX-License-Identifier: GPL-3.0-only

add_library(openbci STATIC
	openbci.c
	openbci.h
	commands.c
	packets.c
	OpenBCI_32bit_Library_Definitions.h
)

target_link_libraries(openbci PRIVATE med)

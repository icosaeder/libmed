# SPDX-License-Identifier: GPL-3.0-only

add_library(openbci STATIC
	openbci.c
	openbci.h
	commands.c
	impedance.c
	packets.c
	OpenBCI_32bit_Library_Definitions.h
)

target_link_libraries(openbci PRIVATE med)

find_library(MATH_LIBRARY m)
if(MATH_LIBRARY)
	target_link_libraries(dummy PRIVATE ${MATH_LIBRARY})
endif()


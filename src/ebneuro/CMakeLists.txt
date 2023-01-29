# SPDX-License-Identifier: GPL-3.0-only

add_library(ebneuro STATIC
	ebneuro.c
	ebneuro.h
	network.c
	packets.h
)

target_link_libraries(ebneuro PRIVATE med)
target_link_libraries(ebneuro PRIVATE system)


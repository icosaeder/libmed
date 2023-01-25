# SPDX-License-Identifier: GPL-3.0-only

add_library(dummy STATIC
	dummy.c
	${HEADER_FILES}
)
target_include_directories(dummy PRIVATE ../../include)
target_link_libraries(dummy m)


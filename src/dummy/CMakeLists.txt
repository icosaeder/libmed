# SPDX-License-Identifier: GPL-3.0-only

add_library(dummy STATIC
	dummy.c
)

target_link_libraries(dummy PRIVATE med)

find_library(MATH_LIBRARY m)
if(MATH_LIBRARY)
	target_link_libraries(dummy PRIVATE ${MATH_LIBRARY})
endif()


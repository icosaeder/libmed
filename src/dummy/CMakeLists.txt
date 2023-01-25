# SPDX-License-Identifier: GPL-3.0-only

add_library(dummy STATIC
	dummy.c
	${HEADER_FILES}
)
target_include_directories(dummy PRIVATE ../../include)

find_library(MATH_LIBRARY m)
if(MATH_LIBRARY)
	target_link_libraries(dummy PUBLIC ${MATH_LIBRARY})
endif()


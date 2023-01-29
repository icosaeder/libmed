# SPDX-License-Identifier: GPL-3.0-only

add_executable(meddump
	meddump.c
	${HEADER_LIST}
)

target_include_directories(meddump PRIVATE ../include)
target_link_libraries(meddump PRIVATE libmed)

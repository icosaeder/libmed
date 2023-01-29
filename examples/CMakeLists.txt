# SPDX-License-Identifier: GPL-3.0-only

add_executable(med_dummy
	dummy.c
)

target_include_directories(med_dummy PRIVATE ../include)
target_link_libraries(med_dummy PRIVATE med)

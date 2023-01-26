# SPDX-License-Identifier: GPL-3.0-only

set(HEADER_LIST "${libmed_SOURCE_DIR}/include/med/eeg.h")

add_library(libmed
	eeg.c
	drivers.h
	include/med/eeg_priv.h
	${HEADER_LIST}
)

target_include_directories(libmed PUBLIC ../include)
target_include_directories(libmed PUBLIC include)

add_subdirectory(system)
target_link_libraries(libmed PUBLIC system)

add_subdirectory(dummy)
add_subdirectory(ebneuro)

target_link_libraries(libmed PRIVATE
	dummy
	ebneuro
)

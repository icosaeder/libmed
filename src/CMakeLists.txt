# SPDX-License-Identifier: GPL-3.0-only

set(HEADER_LIST "${libmed_SOURCE_DIR}/include/med/eeg.h")

add_library(med
	eeg.c
	drivers.h
	include/med/eeg_priv.h
	${HEADER_LIST}
)

target_include_directories(med PUBLIC ../include)
target_include_directories(med PUBLIC include)

add_subdirectory(system)
target_link_libraries(med PUBLIC system)

add_subdirectory(dummy)
add_subdirectory(ebneuro)
add_subdirectory(openbci)

target_link_libraries(med PRIVATE
	dummy
	ebneuro
	openbci
)

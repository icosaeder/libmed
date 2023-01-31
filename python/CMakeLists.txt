# SPDX-License-Identifier: GPL-3.0-only

find_package(SWIG REQUIRED)
include(${SWIG_USE_FILE})

find_package(Python3 COMPONENTS Interpreter NumPy REQUIRED)
set(CMAKE_SWIG_FLAGS "")

swig_add_library(pylibmed LANGUAGE python SOURCES
	libmed.i
	${HEADER_LIST}
)
set_property(TARGET pylibmed PROPERTY SWIG_USE_TARGET_INCLUDE_DIRECTORIES TRUE)

target_include_directories(pylibmed PRIVATE ${PYTHON_INCLUDE_DIRS})
target_include_directories(pylibmed PRIVATE ../include)

swig_link_libraries(pylibmed med ${PYTHON_LIBRARIES} Python3::NumPy)




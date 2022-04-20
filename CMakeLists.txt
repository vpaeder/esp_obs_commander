# The following lines of boilerplate have to be in your project's CMakeLists
# in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.5)

set(EXTRA_COMPONENT_DIRS "components/lvglpp/components")
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CXX_FLAGS "-Wdocumentation")

set(PYTHON_EXECUTABLE "$ENV{IDF_PYTHON_ENV_PATH}/bin/python")

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(esp_obs_commander)

# Documentation
FIND_PACKAGE(Doxygen)
IF (DOXYGEN_FOUND)
	option(BUILD_DOC "Build documentation" ON)
ENDIF()

IF (DOXYGEN_FOUND AND BUILD_DOC)
  set(DOXYGEN_IN ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in)
  set(DOXYGEN_OUT ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)
	configure_file(${DOXYGEN_IN} ${DOXYGEN_OUT} @ONLY)
	add_custom_target( doc_doxygen ALL
		COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT}
		WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
		COMMENT "Generating API documentation with Doxygen"
		VERBATIM )
ENDIF()

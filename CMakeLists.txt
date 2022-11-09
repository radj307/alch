# alch/
cmake_minimum_required (VERSION 3.20)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/307lib/307modules")

set(ENV{alch_VERSION} "0.0.0")
include(VersionTag)
GET_VERSION_TAG("${CMAKE_CURRENT_SOURCE_DIR}" "alch")

project ("alch" VERSION "${alch_VERSION}" LANGUAGES CXX)

option(BUILD_ALCH2 "Build the 'alch2' target instead of the 'alch' target." ON)

add_subdirectory("307lib")
if (BUILD_ALCH2)
	# alch2
	add_subdirectory ("alchlib2")
	add_subdirectory ("alch2")
else()
	# alch
	add_subdirectory ("alchlib")
	add_subdirectory ("alch")
endif()

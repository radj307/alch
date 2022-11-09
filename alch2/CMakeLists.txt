# alch/alch2
cmake_minimum_required(VERSION 3.22)

file(GLOB HEADERS
	RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}"
	CONFIGURE_DEPENDS
	"*.h*"
)
file(GLOB SRCS
	RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}"
	CONFIGURE_DEPENDS
	"*.c*"
)

file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/rc")

include(CopyrightMaker)
MAKE_COPYRIGHT_NOTICE(_alch2_copyright "radj307" "2022")

if (WIN32)
	include(ResourceMaker)
	set(alch_rc_path "${CMAKE_CURRENT_BINARY_DIR}/rc/alch.rc")

	MAKE_STRINGRC_VERSIONINFO(
		alch_stringrc_versioninfo
		"${alch_VERSION}"
		"Copyright © 2022 by ${ALCH2_AUTHOR}"
		"radj307"
		"alch"
		"Skyrim SE alchemy commandline utility."
	)

	MAKE_RESOURCE("${alch_rc_path}" "${alch_stringrc_versioninfo}")
endif()

MAKE_VERSION_HEADER("${CMAKE_CURRENT_BINARY_DIR}/rc/version.h" "alch" "${alch_VERSION}")
MAKE_COPYRIGHT_HEADER_FROM_NOTICE("${CMAKE_CURRENT_BINARY_DIR}/rc/copyright.h" "${_alch2_copyright}")

add_executable(alch2 "${SRCS}" "${alch_rc_path}" "${CMAKE_CURRENT_BINARY_DIR}/rc/version.h" "${CMAKE_CURRENT_BINARY_DIR}/rc/copyright.h")

set_property(TARGET alch2 PROPERTY CXX_STANDARD 20)
set_property(TARGET alch2 PROPERTY CXX_STANDARD_REQUIRED ON)

target_compile_options(alch2 PRIVATE "${307lib_compiler_commandline}")

target_sources(alch2 PRIVATE "${HEADERS}")

target_include_directories(alch2 PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/rc")

target_link_libraries(alch2 PRIVATE shared TermAPI filelib alchlib2)

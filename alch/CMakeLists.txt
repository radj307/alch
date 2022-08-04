# alch/alch
cmake_minimum_required(VERSION 3.20)

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

file(MAKE_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/rc")

if (WIN32)
	include(ResourceMaker)
	set(alch_rc_path "${CMAKE_CURRENT_SOURCE_DIR}/rc/alch.rc")

	MAKE_STRINGRC_VERSIONINFO(alch_stringrc_versioninfo
		"${alch_VERSION}"
		"GNU General Public License v3 @ radj307"
		"radj307"
		"alch"
		"Skyrim SE alchemy commandline utility."
	)

	MAKE_RESOURCE("${alch_rc_path}" "${alch_stringrc_versioninfo}")
endif()

include(VersionTag)
MAKE_VERSION_HEADER("${CMAKE_CURRENT_SOURCE_DIR}/rc/version.h" "alch" "${alch_VERSION}")

add_executable(alch "${SRCS}" "rc/version.h" "${alch_rc_path}")

set_property(TARGET alch PROPERTY CXX_STANDARD 20)
set_property(TARGET alch PROPERTY CXX_STANDARD_REQUIRED ON)

if (MSVC)
	target_compile_options(alch PRIVATE "/Zc:__cplusplus" "/Zc:preprocessor")
endif()

target_sources(alch PRIVATE "${HEADERS}")

target_link_libraries(alch PUBLIC alchlib TermAPI optlib)

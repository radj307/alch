# alch/alchlib
cmake_minimum_required (VERSION 3.20)

file(GLOB HEADERS
	RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}"
	CONFIGURE_DEPENDS
	"include/*.h*"
)
file(GLOB SRCS
	RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}"
	CONFIGURE_DEPENDS
	"src/*.c*"
)

add_library(alchlib STATIC "${SRCS}")

set_property(TARGET alchlib PROPERTY CXX_STANDARD 20)
set_property(TARGET alchlib PROPERTY CXX_STANDARD_REQUIRED ON)
set_property(TARGET alchlib PROPERTY POSITION_INDEPENDENT_CODE ON)

if (MSVC)
	target_compile_options(alchlib PRIVATE "/Zc:__cplusplus" "/Zc:preprocessor")
endif()

include(PrependEach)
PREPEND_EACH(HEADERS_ABS "${HEADERS}" "${CMAKE_CURRENT_SOURCE_DIR}/")

target_sources(alchlib PUBLIC
	"$<BUILD_INTERFACE:${HEADERS_ABS}>"
	"$<INSTALL_INTERFACE:${HEADERS}>"
)

target_link_libraries(alchlib PUBLIC TermAPI optlib strlib filelib)

target_include_directories(alchlib PUBLIC
	"$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include;${CMAKE_CURRENT_SOURCE_DIR}/src>"
	"$<INSTALL_INTERFACE:include;src>"
)

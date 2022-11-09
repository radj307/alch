# alch/alchlib2
cmake_minimum_required(VERSION 3.22)

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

add_library(alchlib2 STATIC "${SRCS}")

set_property(TARGET alchlib2 PROPERTY CXX_STANDARD 20)
set_property(TARGET alchlib2 PROPERTY CXX_STANDARD_REQUIRED ON)
set_property(TARGET alchlib2 PROPERTY POSITION_INDEPENDENT_CODE ON)
set_property(TARGET alchlib2 PROPERTY WINDOWS_EXPORT_ALL_SYMBOLS ON)

target_compile_options(alchlib2 PRIVATE "${307lib_compiler_commandline}")

include(PrependEach)
PREPEND_EACH(HEADERS_ABS "${HEADERS}" "${CMAKE_CURRENT_SOURCE_DIR}/")

target_sources(alchlib2 PUBLIC 
	"$<BUILD_INTERFACE:${HEADERS_ABS}>"
	"$<INSTALL_INTERFACE:${HEADERS}>"
)

target_include_directories(alchlib2 PUBLIC
	"$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
	"$<INSTALL_INTERFACE:include>"
)

# Setup nlohmann/json
include(FetchContent)
FetchContent_Declare(nlohmann_json
	GIT_REPOSITORY	https://github.com/nlohmann/json.git
	GIT_TAG			v3.11.2
)
FetchContent_MakeAvailable(nlohmann_json)

target_link_libraries(alchlib2 PUBLIC shared strlib nlohmann_json::nlohmann_json)

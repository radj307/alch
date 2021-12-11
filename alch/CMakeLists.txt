# alch executable
cmake_minimum_required(VERSION 3.15)

set(CMAKE_CXX_STANDARD 20)

if (MSVC)
	set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
	# Fix MSVC __cplusplus macro
	string(APPEND CMAKE_CXX_FLAGS " /Zc:__cplusplus")
endif()

project("alch")

set(HEADERS
	"Instance.hpp"
)

set(SRC
	"main.cpp"
)

add_executable(alch ${HEADERS} ${SRC})

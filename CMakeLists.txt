cmake_minimum_required (VERSION 3.15)

set(CMAKE_CXX_STANDARD 20)

if (MSVC)
	set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
	# Fix MSVC __cplusplus macro
	string(APPEND CMAKE_CXX_FLAGS " /Zc:__cplusplus")
endif()

project ("alch")

# Add executable
add_subdirectory ("alch")

# Add dependencies
set(TERMAPI_VERSION_3 ON)
set(FILELIB_ENABLE_EXTENSIONS OFF)
add_subdirectory("307lib")

# Add alch-lib
add_subdirectory ("alch-lib")

target_link_libraries(alch PUBLIC shared str-lib TermAPI file-lib opt-lib alch-lib)

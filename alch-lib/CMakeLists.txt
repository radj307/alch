# alch-lib
cmake_minimum_required (VERSION 3.15)

set(CMAKE_CXX_STANDARD 20)

if (MSVC)
	set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
	# Fix MSVC __cplusplus macro
	string(APPEND CMAKE_CXX_FLAGS " /Zc:__cplusplus")
endif()

project("alch-lib")

set(HEADERS
	"./include/using.h"
	"./include/GameConfig.hpp"
	"./include/ColorAPI.hpp"
	"./include/reparse.hpp"
	"./include/reloader.hpp"

	"./include/ObjectBase.hpp"
	"./include/Keyword.hpp"
	"./include/Effect.hpp"
	"./include/Ingredient.hpp"
	"./include/Potion.hpp"
	"./include/IngredientCache.hpp"
	
	"./include/OutputFormat.hpp"
	"./include/Format.hpp"

	"./include/Alchemy.hpp"

	"./include/handle_arguments.hpp"
	"./include/DefaultObjects.hpp"
)
set(SRC
	"./src/DefaultObjects.cpp"
)

add_library(alch-lib STATIC ${HEADERS} ${SRC})
target_link_libraries(alch-lib PUBLIC shared TermAPI strlib filelib optlib)
target_include_directories(alch-lib PUBLIC "./include" "./src")


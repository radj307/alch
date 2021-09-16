#pragma once
#include "pch.h"
#include "CppUnitTest.h"
#include "..\alch_win\init.hpp"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace caco_alch;

#pragma region GLOBALS
const std::string local_path{ "X:\\_Workspace\\_repo\\alch\\Debug\\" }; // default local path (location of INI, GMST cfg, and Ingredient Cache)

const DefaultPaths paths{ // default paths
	local_path,
	DefaultObjects._default_filename_config,
	DefaultObjects._default_filename_gamesettings,
	DefaultObjects._default_filename_registry
};

inline void compare_sstream(std::stringstream& expected, std::stringstream& buffer, const wchar_t* message = NULL, const __LineInfo* pLineInfo = NULL)
{
	Assert::AreEqual(expected.str(), buffer.str());
	Assert::AreEqual(expected.rdbuf()->view(), buffer.rdbuf()->view()); // compare strings
}

inline std::streambuf* swapBuffer(std::streambuf* sbuf)
{
	std::cout.rdbuf(sbuf);
	return sbuf;
}
#pragma endregion GLOBALS

#pragma region TEST_FUNCTIONS
inline int test_search()
{
	std::stringstream expected{ // EXPECTED OUTPUT
		file::read("expected-search.txt")
	};
	opt::Params args(opt::ContainerType{ // ARGUMENTS TO INVOKE EXPECTED OUTPUT
		{ "-s" },
		{ "root" },
		{ "Jarrin" }
	});

	std::stringstream buffer;
	auto sbuf{ swapBuffer(buffer.rdbuf()) }; // swap std::cout with buffer

	int result{ handle_arguments(init(args, paths)) }; // get the result of the test

	swapBuffer(sbuf); // swap std::cout with sbuf (reset)

	compare_sstream(expected, buffer); // run test comparison using expected buffer & output buffer

	return result;
}

inline int test_build()
{
	std::stringstream expected{ // EXPECTED OUTPUT
		file::read("expected-build.txt")
	};
	opt::Params args(opt::ContainerType{ // ARGUMENTS TO INVOKE EXPECTED OUTPUT
		{ "-b" },
		{ "jarrin root" },
		{ "crimson nirnroot" },
		{ "funnel cap" }
	});

	std::stringstream buffer;
	auto sbuf{ swapBuffer(buffer.rdbuf()) }; // swap std::cout with buffer

	int result{ handle_arguments(init(args, paths)) }; // get the result of the test

	swapBuffer(sbuf); // swap std::cout with sbuf (reset)

	compare_sstream(expected, buffer); // run test comparison using expected buffer & output buffer

	return result;
}

inline int test_ssearch()
{
	std::stringstream expected{ // EXPECTED OUTPUT
		file::read("expected-smart-search.txt")
	};
	opt::Params args(opt::ContainerType{ // ARGUMENTS TO INVOKE EXPECTED OUTPUT
		{ "-S" },
		{ "damage health" },
		{ "damage stamina" },
	});

	std::stringstream buffer;
	auto sbuf{ swapBuffer(buffer.rdbuf()) }; // swap std::cout with buffer

	int result{ handle_arguments(init(args, paths)) }; // get the result of the test

	swapBuffer(sbuf); // swap std::cout with sbuf (reset)

	compare_sstream(expected, buffer); // run test comparison using expected buffer & output buffer

	return result;
}

inline int test_list_all()
{
	std::stringstream expected{ // EXPECTED OUTPUT
		file::read("expected-list-all.txt")
	};
	opt::Params args(opt::ContainerType{ // ARGUMENTS TO INVOKE EXPECTED OUTPUT
		{ "-la" },
	});

	std::stringstream buffer;
	auto sbuf{ swapBuffer(buffer.rdbuf()) }; // swap std::cout with buffer

	int result{ handle_arguments(init(args, paths)) }; // get the result of the test

	swapBuffer(sbuf); // swap std::cout with sbuf (reset)

	compare_sstream(expected, buffer); // run test comparison using expected buffer & output buffer

	return result;
}

inline int test_search_exact()
{
	std::stringstream expected{ // EXPECTED OUTPUT
		file::read("expected-search-exact.txt")
	};
	opt::Params args(opt::ContainerType{ // ARGUMENTS TO INVOKE EXPECTED OUTPUT
		{ "-se" },
		{ "nirnroot" },
		{ "paralysis" },
	});

	std::stringstream buffer;
	auto sbuf{ swapBuffer(buffer.rdbuf()) }; // swap std::cout with buffer

	int result{ handle_arguments(init(args, paths)) }; // get the result of the test

	swapBuffer(sbuf); // swap std::cout with sbuf (reset)

	compare_sstream(expected, buffer); // run test comparison using expected buffer & output buffer

	return result;
}

inline int test_search_verbose()
{
	std::stringstream expected{ // EXPECTED OUTPUT
		file::read("expected-search-verbose.txt")
	};
	opt::Params args(opt::ContainerType{ // ARGUMENTS TO INVOKE EXPECTED OUTPUT
		{ "-sv" },
		{ "nirnroot" },
	});

	std::stringstream buffer;
	auto sbuf{ swapBuffer(buffer.rdbuf()) }; // swap std::cout with buffer

	int result{ handle_arguments(init(args, paths)) }; // get the result of the test

	swapBuffer(sbuf); // swap std::cout with sbuf (reset)

	compare_sstream(expected, buffer); // run test comparison using expected buffer & output buffer

	return result;
}
#pragma endregion TEST_FUNCTIONS

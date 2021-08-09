/**
 * @file main.cpp
 * @author radj307
 * @brief Program entry point.
 */

//#define DEBUG_SWITCH ///< @brief Define this in debug configuration to switch to debug mode.
#if defined(DEBUG_SWITCH) && defined(_DEBUG)
#define ENABLE_DEBUG
#include <xRand.h>
#endif

#include <optlib.hpp>
#include <optenv.hpp>
#include <sysapi.h>
#include <xINI.hpp>
#include "reloader.hpp"
 //#include "Loader.hpp"
#include "Alchemy.hpp"
#include "UserAssist.hpp"

#include "GameSettings.hpp"

using namespace caco_alch;

std::tuple<opt::Param, Alchemy, GameSettings> init(const int argc, char* argv[]);


/**
 * @brief Main.
 * @param argc	- (implicit)
 * @param argv	- (implicit)
 * @returns int
 *			1	- No valid commandline options were found. \n
 *			0	- Successful execution. \n
 *			-1	- An exception occurred and the program performed a controlled crash. \n
 *			-2	- An unknown exception occurred and the program performed a controlled crash. \n
 */
int main(const int argc, char* argv[])
{
	// TODO:
	// Implement duration processing in Potion.hpp now that reparse is being used for the registry.
	// Implement '-r' recursive search (incompatible with '-e'), allow matches for any individual word
	// Implement "-R" arg to reverse sorted output
	// Implement the ability to change INI settings from the commandline (Ex: alchemy skill level, duration)
	try
	{
#ifndef ENABLE_DEBUG
		return handle_arguments(init(argc, argv));
#else
		dRand rng;
		auto pr{ init(argc, argv) };
		return handle_arguments(std::move(pr));
#endif
	} catch ( std::exception& ex ) {
		std::cout << sys::error << ex.what() << std::endl;
		return -1;
	} catch ( ... ) {
		std::cout << sys::error << "An unknown exception occurred." << std::endl;
		return -2;
	}
}

/**
 * @function init(const int, char*[])
 * @brief Initialize the program and its assets, process interrupt opts.
 * @param argc	- See main()
 * @param argv	- See main()
 * @returns std::pair<opt::Param, Alchemy>
 */
std::tuple<opt::Param, Alchemy, GameSettings> init(const int argc, char* argv[])
{
	// parse arguments
	opt::Param args(argc, argv, _matcher);
#ifndef ENABLE_DEBUG
	if ( args.size(true) == 0 ) { // print help if no valid parameters found.
		Help::print();
		throw std::exception("No valid parameters detected.");
	}
#endif
	if ( args.getFlag('C') ) {
		std::stringstream ss;
		ss << std::cin.rdbuf();
		for ( std::string ln{ }; std::getline(ss, ln);  ) {

		}
	}
	if ( args.getFlag('h') )
		Help::print();
	// resolve file target for ingredient registry & INI
	const auto loc{ [&argv]() -> std::pair<std::string, std::string> {
		const std::string loc{ argv[0] };
		const auto dPos{ loc.find_last_of("/\\") };
		if ( dPos != std::string::npos )
			return { loc.substr(0, dPos), loc.substr(dPos + 1) };
		return { };
	}( ) };
	const std::string
		_DEF_INI{ [&loc]() -> std::string { const auto dPos{ loc.second.rfind('.') }; if ( dPos != std::string::npos ) return loc.second.substr(0, dPos) + ".ini"; return { }; }( ) },
		_DEF_FILE{ loc.first + '\\' + "caco-ingredient-list.dat" }; ///< @brief Replace the exe name in loc with the default name of the ingredient registry file.
	const std::string filename{ [&args, &_DEF_FILE]() -> std::string {
		std::string ret{ args.getv("load") };
		if ( ret.empty() )
			ret = _DEF_FILE;
		return ret;
	}( ) }, ini_filename{ [&args, &_DEF_INI]() -> std::string {
		std::string ret{ args.getv("ini") };
		if ( ret.empty() )
			ret = _DEF_INI;
		return ret;
	}( ) };

	if ( args.check_opt("validate") ) // Process "--validate" opt
		if ( !static_cast<bool>( validate_file(filename) ) )
			throw std::exception("VALIDATE_FAILED"); // throw exception to break from handle_arguments

	if ( args.check_opt("ini-reset") ) {
		write_ini(ini_filename);
		exit(0);
	}

	GameSettings gs{ };
	if ( file::exists(ini_filename) )
		gs = read_ini(ini_filename);

	// Check for stat mods
	bool do_write_ini{ false };
	if ( args.check_opt("ini-modav-alchemy") ) {
		gs._alchemy_skill = [&args]() { const auto v{ str::stoui(args.getv("ini-modav-alchemy")) }; if ( v != 0u ) return v; return 15u; }( );
		do_write_ini = true;
	}
	if ( args.check_opt("ini-default-duration") ) {
		gs._duration = [&args]() { const auto v{ str::stoui(args.getv("ini-default-duration")) }; if ( v != 0u ) return v; return 15u; }( );
		do_write_ini = true;
	}
	if ( do_write_ini ) {
		write_ini(ini_filename, gs);
		exit(0);
	}

	// Init alchemy instance
	auto alch{ Alchemy(loadFromFile(filename)) };

	return { args, alch, gs };
}
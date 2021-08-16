/**
 * @file main.cpp
 * @author radj307
 * @brief Program entry point.
 */

#define DEBUG_SWITCH ///< @brief Define this in debug configuration to switch to debug mode.
#if defined(DEBUG_SWITCH) && defined(_DEBUG)
#define ENABLE_DEBUG
#include <xRand.h>
#endif

#include <optlib.hpp>
#include <optenv.hpp>
#include <sysapi.h>
#include <xINI.hpp>
#include "reloader.hpp"
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
	// Implement logical operators to display results that have multiple traits.
	// Implement alternative sorting algorithms for SortedIngrList container, for example to sort by magnitude or duration.
	// Implement "-R" arg to reverse sorted output
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

GameSettings handle_ini_opts(const std::string& ini_filename, opt::Param& args)
{
	GameSettings gs; // INIT
	if ( !file::exists(ini_filename) && !args.check_opt("ini-reset") ) return gs; // return early

	else if ( file::exists(ini_filename) ) {// read the INI file if it exists
		gs = read_ini(ini_filename); // overwrite default game settings
		#ifdef ENABLE_DEBUG
			std::cout << sys::log << "Successfully read game settings from INI file \"" << ini_filename << '\"' << std::endl;
		#endif
	}

	bool update_ini_before_return{ false };

	if ( args.check_opt("ini-reset") ) { // check if user wants to reset/create INI config file.
		if ( write_ini(ini_filename, gs) )
			std::cout << sys::msg << "Successfully wrote to \"" << ini_filename << '\"' << std::endl;
		else std::cout << sys::warn << "Failed to write to \"" << ini_filename << '\"' << std::endl;
	}

	if ( args.check_opt("ini-alchemy-skill") ) { // check if user wants to change their alchemy skill level
		try {
			if ( const auto skill{ std::stod(args.getv("ini-alchemy-skill")) }; skill != gs.AlchemyAV() ) {
				if ( gs.set_AlchemyAV(skill) )
					std::cout << sys::msg << "Changed alchemy skill level to " << Color::f_cyan << skill << Color::reset << '\n';
				update_ini_before_return = true;
			}
			else
				std::cout << sys::msg << "Alchemy skill level was already set to " << Color::f_cyan << skill << Color::reset << '\n';
		} catch ( std::exception& ex ) {
			std::cout << sys::error << "An exception occurred while changing the alchemy skill level: \"" << ex.what() << '\"' << std::endl;
		}
	}
	if ( args.check_opt("ini-alchemy-mod") ) {
		try {
			if ( const auto mod{ std::stod(args.getv("ini-alchemy-mod")) }; mod != gs.AlchemyMod() ){
				if ( gs.set_AlchemyMod(mod) )
					std::cout << sys::msg << "Changed alchemy skill modifier to " << Color::f_green << mod << Color::reset << '\n';
				update_ini_before_return = true;
			}
			else
				std::cout << sys::msg << "Alchemy skill modifier was already set to " << Color::f_green << mod << Color::reset << '\n';
		} catch ( std::exception& ex ) {
			std::cout << sys::error << "An exception occurred while changing the alchemy skill modifier: \"" << ex.what() << '\"' << std::endl;
		}
	}

	if ( update_ini_before_return )
		write_ini(ini_filename, gs, false);

	return gs; // RETURN
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
	opt::Param args(argc, argv, std::move(_matcher));
#ifndef ENABLE_DEBUG
	if ( args.empty() ) { // print help if no valid parameters found.
		Help::print();
		throw std::exception("No valid parameters detected.");
	}
#endif
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
		_DEF_FILE{ loc.first.empty() ? "caco-ingredient-list.dat" : loc.first + '\\' + "caco-ingredient-list.dat" }; ///< @brief Replace the exe name in loc with the default name of the ingredient registry file.
	const std::string filename{ [&args, &_DEF_FILE]() -> std::string {
		std::string ret{ args.getv("load") };
		if ( ret.empty() )
			ret = _DEF_FILE;
		return ret;
	}( ) }, ini_filename{ [&args, &_DEF_INI]() -> std::string {
		std::string ret{ args.getv("ini") };
		if ( ret.empty() )
			ret = "X:\\bin\\alch.ini"; // TEMP DEBUG FIX
		//	ret = _DEF_INI; // DEBUG
		return ret;
	}( ) };

	if ( args.check_opt("validate") ) { // Process "--validate" opt
		std::cout << "argv[0]" << std::setw(12u - 7u) << ' ' << argv[0] << std::endl;
		std::cout << "filename" << std::setw(12u - 8u) << ' ' << filename << std::endl;
		if ( validate_file(filename) )
			std::cout << sys::msg << "Validation succeeded." << std::endl;
		else std::cout << sys::warn << "Validation failed." << std::endl;
	}


	return { args, Alchemy(loadFromFile(filename)), handle_ini_opts(ini_filename, args) };
}
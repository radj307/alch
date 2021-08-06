/**
 * @file main.cpp
 * @author radj307
 * @brief Program entry point.
 */
#include <optlib.hpp>
#include <optenv.hpp>
#include <sysapi.h>
#include <xINI.hpp>
#include "Loader.hpp"
#include "Alchemy.hpp"
#include "UserAssist.hpp"

using namespace caco_alch; ///< @brief Contains everything +

/**
 * @brief Main.
 * @param argc	- (implicit)
 * @param argv	- (implicit)
 * @return 1	- No valid commandline options were found.
 * @return 0	- Successful execution.
 * @return -1	- An exception occurred and the program performed a controlled crash.
 * @return -2	- An unknown exception occurred and the program performed a controlled crash.
 */
int main(const int argc, char* argv[])
{
	// TODO:
	// Implement "-b" command, take at least 2 (max 4) ingredient names & display which effects have at least 2 matches, and the result of their magnitudes
	try 
	{
		opt::Param args(argc, argv, _matcher); // parse arguments

		if ( args.size(true) == 0 ) { // print help if no valid parameters found.
			std::cout << sys::error << "No valid parameters found.\n";
			Help::print();
			return 1;
		}

		const auto loc{ [&argv]() -> std::pair<std::string, std::string> {
			const std::string loc{ argv[0] };
			const auto dPos{ loc.find_last_of("/\\") };
			if ( dPos != std::string::npos )
				return { loc.substr(0, dPos), loc.substr(dPos + 1) };
			return { };
		}() };
		const std::string 
			_DEF_INI{ [&loc]() -> std::string { const auto dPos{ loc.second.rfind('.') }; if ( dPos != std::string::npos ) return loc.second.substr(0, dPos) + ".ini"; return { }; }() }, 
			_DEF_FILE{ loc.first + '\\' + "caco-ingredient-list.dat" }; ///< @brief Replace the exe name in loc with the default name of the ingredient registry file.

		// resolve target ingredient registry, and INI
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
		
		if ( args.check_opt("validate") ) // if the --validate option was specified
			return validate_file(filename);

		if ( args.check_opt("write_ini") )
			write_ini(ini_filename, GameSettings());

		const auto gs{ load_ini(ini_filename) }; ///< @brief Get INI if it exists

		// else:
		auto alch{ Alchemy(loadFromFile(filename)) };
		if ( args.check_opt("color") ) // if color override was specified, use it.
			alch._ingr_color = Color::strToColor(args.getv("color"));
		return handle_arguments(std::move(args), std::move(alch));
	} catch ( std::exception& ex ) {
		std::cout << sys::error << "An exception occurred: \"" << ex.what() << '\"' << std::endl;
		return -1;
	} catch ( ... ) {
		std::cout << sys::error << "An unknown exception occurred." << std::endl;
		return -2;
	}
}
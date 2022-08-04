/**
 * @file main.cpp
 * @author radj307
 * @brief Program entry point.
 */
 //#define DEBUG_SWITCH ///< @brief Define this in debug configuration to switch to debug mode.
 // must be in debug configuration, and have DEBUG_SWITCH defined.
#if defined(DEBUG_SWITCH) && defined(_DEBUG)
#define ENABLE_DEBUG
#endif

#include "rc/version.h"
#include "Instance.hpp"

#include <make_exception.hpp>
#include <TermAPI.hpp>
#include <opt3.hpp>
#include <env.hpp>
#include <str.hpp>

struct Help {
	std::string programName;
	WINCONSTEXPR Help(std::string const& programName) : programName{ programName } {}
	friend std::ostream& operator<<(std::ostream& os, const Help& h)
	{
		return os
			<< "alch v" << alch_VERSION_EXTENDED << '\n'
			<< "  Skyrim Alchemy Commandline Utility" << '\n'
			<< '\n'
			<< "USAGE:\n"
			<< "  " << h.programName << " [OPTIONS] [<MODE> [MODIFIERS] [<NAME>...]]" << '\n'
			<< '\n'
			<< "  Only one mode can be specified at a time; to use multiple modes, call the program multiple times." << '\n'
			<< '\n'
			<< "OPTIONS:\n"
			<< "  -h, --help                 Shows this help display, then exits." << '\n'
			<< "      --version              Shows the current version number, then exits." << '\n'
			<< "  -c                         Disables colorization of effect names based on their keywords." << '\n'
			<< "      --precision <#>        Sets the floating-point printing precision to the specified number of digits." << '\n'
			<< "      --validate             Shows debug information including filepaths, and whether they were found or not." << '\n'
			<< "      --ingredients          Specifies the location of the ingredients registry.     (Default is 'alch.ingredients')" << '\n'
			<< "      --config               Specifies the location of the INI config file.          (Default is 'alch.ini')" << '\n'
			<< "      --gamesettings         Specifies the location of the GameSetting config file.  (Default is 'alch.gamesettings')" << '\n'
			<< "      --reset                Resets the GameSettings config file to default, or creates a new one if it doesn't exist." << '\n'
			<< "      --set <SETTING:VALUE>  Set the specified GameSetting to the given value." << '\n'
			<< "      --get <SETTING>        Gets the current value of the specified GameSetting" << '\n'
			<< '\n'
			<< "MODES:\n"
			<< "  -l                         Lists all ingredients in the registry." << '\n'
			<< "  -s                         Searches for the ingredients or effects specified by '[<NAME>...]'." << '\n'
			<< "  -S                         Searches for effects as specified by '[<NAME>...]' & shows ingredients with those effects." << '\n'
			<< "  -b                         Shows the potion that would result from combining 2-4 specified ingredients." << '\n'
			<< "  -i                         Same as '-b' except receives input from STDIN rather than arguments. (Useful with `cat`)" << '\n'
			<< '\n'
			<< "MODIFIERS:\n"
			<< "  -e                         Only allow exact matches. (This does NOT make matching case-sensitive.)" << '\n'
			<< "  -q                         Minimizes the amount of output by hiding less-important fields." << '\n'
			<< "  -v                         Includes associated keywords when printing effects." << '\n'
			<< "  -a                         Shows all output. (Opposite of the '-q' option)" << '\n'
			<< "  -E                         Print output in the same format as the ingredients registry." << '\n'
			<< "  -R                         Reverses the sorting order used." << '\n'
			<< "  -r                         Sorts effect search results by magnitude OR duration." << '\n'
			<< "  -m                         Sorts effect search results by magnitude, or magnitude AND duration when '-d' is included." << '\n'
			<< "  -d                         Sorts effect search results by duration, or magnitude AND duration when '-m' is included." << '\n'
			;
	}
};

/**
 * @brief		Main.
 * @param argc	(implicit) Argument Count
 * @param argv	(implicit) Arguments
 * @returns		int
 *\n			| Return Value	| Description                                                                 |
 *				| -------------	| --------------------------------------------------------------------------- |
 *				| 1				| No valid commandline options received.                                      |
 *				| 0				| Successful execution.                                                       |
 *				| -1			| An exception occurred and the program performed a controlled crash.         |
 *				| -2			| An unknown exception occurred and the program performed a controlled crash. |
 */
int main(const int argc, char* argv[])
{
	using namespace caco_alch;
	// TODO: Implement unit tests using the following framework:
	//		base:	{ Keyword, Effect, Ingredient, Potion },
	//		io:		{ reparse,  }
	// TODO: Add check & INI value for CACO's locked-duration potions. (1s, 5s, 10s)
	// TODO: Add a "request" system to the potion-building mechanic that allows the user to request an automatically-generated potion of a certain type.
	try {
		std::cout << term::EnableANSI; // enable virtual terminal sequences
		using namespace opt3_literals;
		opt3::ArgManager args{ argc, argv,
			"color",
			"precision"_reqcap,
			"set"_reqcap,
			"get"_reqcap,
			"config"_reqcap,
			"gamesettings"_reqcap,
			"ingredients"_reqcap
		}; // parse arguments

		auto path{ env::PATH() };
		const auto& [programPath, programName] {path.resolve_split(argv[0])};

		// parse interrupting help argument
		if (args.check_any<opt3::Option, opt3::Flag>("help", 'h')) {
			std::cout << Help(programName.generic_string()) << std::endl;
			return 0;
		}
		else if (args.check<opt3::Option>("version")) {
			std::cout << alch_VERSION_EXTENDED << std::endl;
			return 0;
		}


		ConfigPathList paths{
			programPath,
			args.getv<opt3::Option>("config").value_or((programPath / DefaultObjects._default_filename_config).generic_string()),
			args.getv<opt3::Option>("gamesettings").value_or((programPath / DefaultObjects._default_filename_gamesettings).generic_string()),
			args.getv<opt3::Option>("ingredients").value_or((programPath / DefaultObjects._default_filename_registry).generic_string())
		};

		Instance inst{ argv[0], args, paths }; // args are parsed further here

		if (args.check<opt3::Option>("validate")) // Process "--validate" opt
			inst.validate(path);

		const auto rc{ inst.handleArguments() }; // finally, runtime args are parsed here
		if (rc == Instance::RETURN_FAILURE)
			throw make_exception("Nothing to do.");

		return rc;
	} catch (std::exception& ex) {
		std::cerr << term::error << ex.what() << std::endl;
		return -1;
	} catch (...) {
		std::cerr << term::crit << "An unknown exception occurred." << std::endl;
		return -2;
	}
}
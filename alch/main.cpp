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

#include "Instance.hpp"

#include <make_exception.hpp>
#include <TermAPI.hpp>
#include <ParamsAPI2.hpp>
#include <env.hpp>
#include <str.hpp>

using namespace caco_alch;

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
	// TODO: Implement unit tests using the following framework:
	//		base:	{ Keyword, Effect, Ingredient, Potion },
	//		io:		{ reparse,  }
	// TODO: Add check & INI value for CACO's locked-duration potions. (1s, 5s, 10s)
	// TODO: Add a "request" system to the potion-building mechanic that allows the user to request an automatically-generated potion of a certain type.
	try {
		std::cout << term::EnableANSI; // enable virtual terminal sequences
		opt::ParamsAPI2 args(argc, argv, "color", "precision", DefaultObjects._set_gamesetting, DefaultObjects._get_gamesetting, DefaultObjects._load_config, DefaultObjects._load_gamesettings, DefaultObjects._load_registry); // parse arguments

		// initialize the path environment variable
		env::PATH path;

		// parse interrupting help argument
		if (args.check_any<opt::Option, opt::Flag>("help", 'h')) {
			std::cout << Help(path.resolve_split(args.arg0().value_or("")).second.replace_extension("").generic_string()) << std::endl;
			return 0;
		}

		const auto getOptOrDefault{ [&args](const std::string& optname, const std::string& defv) {
			const auto val{ args.typegetv<opt::Option>(optname) };
			if (val.has_value())
				return val.value();
			return defv;
		} };

		ConfigPathList paths(
			path.resolve_split(argv[0]).first.generic_string(),
			getOptOrDefault(DefaultObjects._load_config, DefaultObjects._default_filename_config),
			getOptOrDefault(DefaultObjects._load_gamesettings, DefaultObjects._default_filename_gamesettings),
			getOptOrDefault(DefaultObjects._load_registry, DefaultObjects._default_filename_registry)
		);

		Instance inst{ args, paths }; // args are parsed further here

		if (args.check<opt::Option>("validate")) // Process "--validate" opt
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
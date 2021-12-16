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
 * @brief Main.
 * @param argc	- (implicit) Argument Count
 * @param argv	- (implicit) Arguments
 * @param envp	- (implicit) Environment
 * @returns int
 *			1	- No valid commandline options were found. \n
 *			0	- Successful execution. \n
 *			-1	- An exception occurred and the program performed a controlled crash. \n
 *			-2	- An unknown exception occurred and the program performed a controlled crash. \n
 */
int main(const int argc, char* argv[], char* envp[])
{
	// TODO: Add check & INI value for CACO's locked-duration potions. (1s, 5s, 10s)
	// TODO: Add a "request" system to the potion-building mechanic that allows the user to request an automatically-generated potion of a certain type.
	try {
		std::cout << sys::term::EnableANSI; // enable virtual terminal sequences
		opt::ParamsAPI2 args(argc, argv, "color", "precision", DefaultObjects._set_gamesetting, DefaultObjects._load_config, DefaultObjects._load_gamesettings, DefaultObjects._load_registry); // parse arguments

		env::PATH path;

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


		DefaultPaths paths(
			path.resolve_split(argv[0]).first.generic_string(),
			getOptOrDefault(DefaultObjects._load_config, DefaultObjects._default_filename_config),
			getOptOrDefault(DefaultObjects._load_gamesettings, DefaultObjects._default_filename_gamesettings),
			getOptOrDefault(DefaultObjects._load_registry, DefaultObjects._default_filename_registry)
		);

		Instance inst{ args, paths };

		if (args.check<opt::Option>("validate")) // Process "--validate" opt
			inst.validate(path);

		switch (inst.handleArguments()) {
		case Instance::RETURN_FAILURE:
			std::cerr << args << std::endl;
			throw make_exception("No valid arguments found!");
		case Instance::RETURN_SUCCESS:
			return 0;
		default:
			return 1;
		}
	} catch (std::exception& ex) {
		std::cerr << sys::term::error << ex.what() << std::endl;
		return -1;
	} catch (...) {
		std::cerr << sys::term::crit << "An unknown exception occurred." << std::endl;
		return -2;
	}
}
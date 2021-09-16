/**
 * @file main.cpp
 * @author radj307
 * @brief Program entry point.
 */
#define TERMAPI_HPP_FORCE
//#define DEBUG_SWITCH ///< @brief Define this in debug configuration to switch to debug mode.
// must be in debug configuration, and have DEBUG_SWITCH defined.
#if defined(DEBUG_SWITCH) && defined(_DEBUG)
#define ENABLE_DEBUG
#endif

#include <TermAPI.hpp>
#include <CaptureStream.hpp>
#include "UserAssist.hpp"
#include "init.hpp"

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
	// TODO: Implement the 'E' file-export option for build mode
	// TODO: Add a "request" system to the potion-building mechanic that allows the user to request an automatically-generated potion of a certain type.
	// TODO: Implement alternative sorting algorithms for SortedIngrList container, for example to sort by magnitude or duration.
	try {
		std::cout << sys::term::EnableANSI; // enable virtual terminal sequences
		// parse arguments
		opt::Params args(argc, argv, DefaultObjects._matcher);

		const auto local_path{ opt::resolve_split_path(envp, argv[0]).first };

		const auto getOptOrDefault{ [&args](const std::string& optname, const std::string& defv) {
			const auto val{ args.getv(optname) };
			if (val.has_value())
				return val.value();
			return defv;
		} };

		DefaultPaths paths(
			local_path,
			getOptOrDefault(DefaultObjects._load_config, DefaultObjects._default_filename_config),
			getOptOrDefault(DefaultObjects._load_gamesettings, DefaultObjects._default_filename_gamesettings),
			getOptOrDefault(DefaultObjects._load_registry, DefaultObjects._default_filename_registry)
		);

		if (args.check_opt("validate")) // Process "--validate" opt
			validate(args, paths, { argv[0] }, local_path, 16);

		int res{ handle_arguments(init(args, paths)) };
		return res;
	} catch ( std::exception& ex ) {
		std::cout << sys::term::error << ex.what() << std::endl;
		return -1;
	} catch ( ... ) {
		std::cout << sys::term::error << "An unknown exception occurred." << std::endl;
		return -2;
	}
}
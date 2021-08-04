/**
 * @file main.cpp
 * @author radj307
 * @brief Program entry point.
 */
#include <optlib.hpp>
#include <sysapi.h>
#include "Loader.hpp"
#include "Alchemy.hpp"
#include "UserAssist.hpp"

int validate_file(const std::string& filename)
{
	try {
		const auto registry{ loadFromFile(filename) };
		if ( !registry.first.empty() && !registry.second.empty() ) {
			std::cout << sys::msg << "File validation succeeded." << std::endl;
			return 0;
		}
		else throw std::exception();
	} catch ( ... ) {
		std::cout << sys::error << "File validation failed! (Does the file exist, and contain valid data?)" << std::endl;
		return -1;
	}
}

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
	const std::string _DEF_FILE{ "ingredients.txt" };
	try {
		opt::Param args(argc, argv, _matcher); // parse arguments

		if ( args.size(true) == 0 ) { // print help if no valid parameters found.
			std::cout << sys::warn << "No valid parameters found.\n";
			Help::print();
			return 1;
		}

		const std::string filename{ [&args, &_DEF_FILE]() -> std::string {
			std::string ret{ args.getv("load") };
			if ( ret.empty() )
				ret = _DEF_FILE;
			return ret;
		}( ) }; // resolve target filename
		
		if ( args.check_opt("validate") ) // if the --validate option was specified
			return validate_file(filename);
		// else:
		return handle_arguments(std::move(args), Alchemy(loadFromFile(filename)));
	} catch ( std::exception& ex ) {
		std::cout << "An exception occurred: \'" << ex.what() << '\'' << std::endl;
		return -1;
	} catch ( ... ) {
		std::cout << "An unknown exception occurred." << std::endl;
		return -2;
	}
}
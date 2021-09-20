#include <sysapi.h>
#include <optlib.hpp>

#define DEBUG_SWITCH
#if defined(_DEBUG) && defined(DEBUG_SWITCH)
#define ENABLE_DEBUG
#endif

#include "combine.hpp"

using namespace build_registry;

inline void print_help()
{
	std::cout << "USAGE:\n\tbuild-registry --ingredient <ingredient registry filepath> --effect <effect registry filepath>\nOPTIONS:\n\t--ingredient\tSpecifies the filepath to the target ingredient registry.\n\t--effect\tSpecifies the filepath to the target effect registry." << std::endl;
}

int main(const int argc, char* argv[])
{
	try {
		if ( const opt::Param args{ argc, argv, { { }, { { "ingredient", true }, { "effect", true } } } }; args.check_opt("ingredient") && args.check_opt("effect") ) {
			auto ingr{ caco_alch::loadFromFile(args.getv("ingredient")) };
			const Format fmt{ false, false, false, false, true, false };
			fmt.to_fstream(std::cout, merge_effect_keywords(ingr, build_registry::loadFromFile(args.getv("effect"))));
			return 0;
		}
		std::cout << sys::error << "No file targets specified." << std::endl;
		print_help();
		return 1;
	} catch ( std::exception& ex ) {
		std::cout << sys::error << "An exception occurred: \"" << ex.what() << '\"' << std::endl;
		return -1;
	} catch ( ... ) {
		std::cout << sys::error << "An unknown exception occurred." << std::endl;
		return -2;
	}
}
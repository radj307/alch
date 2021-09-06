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
#include <xRand.h>
#endif

#include <TermAPIWin.hpp>
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
		std::cout << sys::win::EnableANSI; // enable virtual terminal sequences
	#ifdef ENABLE_DEBUG // DEBUG MODE
		auto pr{ init(argc, argv, envp) };
		return handle_arguments(std::move(pr));
	#else // RELEASE MODE
		return handle_arguments(init(argc, argv, envp));
	#endif
	} catch ( std::exception& ex ) {
		std::cout << sys::error << ex.what() << std::endl;
		return -1;
	} catch ( ... ) {
		std::cout << sys::error << "An unknown exception occurred." << std::endl;
		return -2;
	}
}
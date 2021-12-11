/**
 * @file UserAssist.hpp
 * @author radj307
 * @brief Contains methods related to user-interaction; such as the Help namespace, and the opt::Matcher instance used to specify valid commandline options.
 */
#pragma once
#include <ParamsAPI2.hpp>
#include <Alchemy.hpp>
#include "reloader.hpp"

namespace caco_alch {
	/**
	 * @function handle_arguments(opt::Param&&, Alchemy&&, GameConfig&&)
	 * @brief Handles primary program execution. Arguments can be retrieved from the init() function.
	 * @param args	- rvalue reference of an opt::Param instance.
	 * @param alch	- Alchemy instance rvalue.
	 * @param gs	- Gamesettings instance rvalue.
	 * @returns int - see main() documentation
	 */
	inline int handle_arguments(opt::ParamsAPI2&& args, Alchemy&& alch)
	{
		if (args.check<opt::Flag>('i')) { // check for receive STDIN flag
			std::stringstream buffer;
			buffer << std::cin.rdbuf();
			alch.print_build(std::cout, parseFileContent(buffer)).flush();
			return 0;
		}

		else if (args.check<opt::Flag>('l')) { // check for list flag
			alch.print_list(std::cout).flush();
			return 0;
		}

		else {
			const auto params{ opt::paramVecToStrVec(args.typeget_all<opt::Parameter>()) };

			if (args.check<opt::Flag>('b')) // Build mode
				alch.print_build(std::cout, params).flush();

			else if (const auto smart{ args.check<opt::Flag>('S') }; args.check<opt::Flag>('s') || smart) { // Search mode
				if (smart)
					alch.print_smart_search(std::cout, params).flush();
				else
					for (auto it{ params.begin() }; it != params.end(); ++it)
						alch.print_search(std::cout, *it).flush();
			}
			return 0;
		}
		return 1;
	}
	// handle_arguments wrapper for TEST project
	inline int handle_arguments(std::tuple<opt::ParamsAPI2, Alchemy>&& pr) { return handle_arguments(std::forward<opt::ParamsAPI2>(std::get<0>(pr)), std::forward<Alchemy>(std::get<1>(pr))); }
}

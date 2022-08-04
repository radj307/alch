/**
 * @file UserAssist.hpp
 * @author radj307
 * @brief Contains methods related to user-interaction; such as the Help namespace, and the opt3::Matcher instance used to specify valid commandline options.
 */
#pragma once
#include "reloader.hpp"
#include "Alchemy.hpp"

#include <opt3.hpp>

namespace caco_alch {
	inline int handle_arguments(opt3::ArgManager&& args, Alchemy&& alch)
	{
		if (args.check<opt3::Flag>('i')) { // check for receive STDIN flag
			std::stringstream buffer;
			buffer << std::cin.rdbuf();
			alch.print_build(std::cout, parseFileContent(buffer)).flush();
			return 0;
		}

		else if (args.check<opt3::Flag>('l')) { // check for list flag
			alch.print_list(std::cout).flush();
			return 0;
		}

		else {
			const auto params{ args.getv_all<opt3::Parameter>() };

			if (args.check<opt3::Flag>('b')) // Build mode
				alch.print_build(std::cout, params).flush();

			else if (const auto smart{ args.check<opt3::Flag>('S') }; args.check<opt3::Flag>('s') || smart) { // Search mode
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
	inline int handle_arguments(std::tuple<opt3::ArgManager, Alchemy>&& pr)
	{
		return handle_arguments(std::forward<opt3::ArgManager>(std::get<0>(pr)), std::forward<Alchemy>(std::get<1>(pr)));
	}
}

/**
 * @file UserAssist.hpp
 * @author radj307
 * @brief Contains methods related to user-interaction; such as the Help namespace, and the opt::Matcher instance used to specify valid commandline options.
 */
#pragma once
#include <map>
#include <string>
#include <iostream>
#include <iomanip>
#include <optlib.hpp>

namespace caco_alch {
	/**
	 * @function handle_arguments(opt::Param&&)
	 * @brief Handles program execution.
	 * @param args	- rvalue reference of an opt::Param instance.
	 * @return 0	- Successful execution.
	 */
	inline int handle_arguments(opt::Param&& args, Alchemy&& alch)
	{
		if ( args.getFlag('l') )
			alch.print_list_to(std::cout, args.getFlag('a'), args.getFlag('q'), args.getFlag('v'));
		if ( args.getFlag('s') )
			for ( auto& it : args._param )
				alch.print_search_to(std::cout, it, args.getFlag('q'), args.getFlag('v'));
		//std::cout << Color::f_green << "Search results for: \'" << it << "'\n" << Color::f_red << "{\n" << Color::reset << alch.search(it, args.getFlag('q'), args.getFlag('v')).rdbuf() << Color::f_red << "}" << Color::reset << std::endl;
		if ( args.getFlag('i') ) {
			std::cout << sys::msg << "[Not Implemented]" << std::endl;
			// TODO: implement interactive mode
		}
		else if ( args.getFlag('b') ) {
			std::cout << sys::msg << "[Not Implemented]" << std::endl;
			// TODO: implement build mode
		}
		return 0;
	}

	// @brief Contains the list of valid commandline arguments for the alch program.
	inline const opt::Matcher _matcher{ { 'l', 's', 'a', 'h', 'i', 'q', 'v', 'b' }, { { "load", true }, { "validate", false }, { "color", true }, { "ini", true } } };

	/**
	 * @namespace Help
	 * @brief Contains methods related to the inline terminal help display.
	 */
	namespace Help {
		/**
		 * @struct Helper
		 * @brief Provides a convenient extensible help display.
		 */
		struct Helper {
			std::string _usage;
			std::unordered_map<std::string, std::string> _doc;
			/**
			 * @constructor Helper(const std::string&, std::map<std::string, std::string>&&)
			 * @brief Default constructor, takes a string containing usage instructions, and a map of all commandline parameters and a short description of them.
			 * @param usage_str	- Brief string showing the commandline syntax for this program.
			 * @param doc		- A map where the key represents the commandline option, and the value is the documentation for that option.
			 */
			Helper(const std::string& usage_str, std::unordered_map<std::string, std::string>&& doc) : _usage{ usage_str }, _doc{ std::move(doc) } { validate(); }

			/**
			 * @function validate()
			 * @brief Called at construction time, iterates through _doc and corrects any abnormalities.
			 */
			void validate()
			{
				for ( auto it{ _doc.begin() }; it != _doc.end(); ++it ) {
					auto& key{ it->first };
					auto& doc{ it->second };
					if ( key.empty() ) // If key is empty, delete entry
						_doc.erase(it);
					else { // key isn't empty
						const auto fst_char{ key.at(0) };
						if ( fst_char != '-' ) { // if key does not have a dash prefix
							const auto tmp = *it;
							_doc.erase(it);
							std::string mod{ '-' };
							if ( key.size() > 1 ) // is longopt
								mod += '-';
							_doc[mod + tmp.first] = tmp.second;
						}
					}
				}
			}

			/**
			 * @function operator<<(std::ostream&, const Helper&)
			 * @brief Stream insertion operator, returns usage string and documentation for all options.
			 * @param os	- (implicit) Output stream instance.
			 * @param h		- (implicit) Helper instance.
			 * @returns std::ostream&
			 */
			friend std::ostream& operator<<(std::ostream& os, const Helper& h)
			{
				os << "Usage:\n  " << h._usage << "\nOptions:\n";
				size_t indent{ 0 };
				for ( auto& it : h._doc )
					if ( const auto size{ it.first.size() }; size > indent )
						indent = size;
				for ( auto& [key, doc] : h._doc )
					os << "  " << std::left << std::setw(static_cast<std::streamsize>( indent ) + 2ll) << key << doc << '\n';
				os.flush();
				return os;
			}
		};

		const Helper _default_doc("caco-alch <[options] [target]>", {
			{ "-h", "Shows this help display." },
			{ "-l", "List all ingredients." },
			{ "-a", "Lists all ingredients and a list of all known effects." },
			{ "-s", "Searches the ingredient & effect lists for all additional parameters, and prints a result to STDOUT" },
			{ "-i", "(Not implemented) Interactive potion-builder mode." },
			{ "-q", "Quiet output, only shows effects that match the search string in search results." },
			{ "-v", "Verbose output, shows magnitude for ingredient effects." },
			{ "-b", "(Incompatible with -s) Build mode, accepts up to 4 ingredient names and shows the result of combining them." },
			{ "--load <file>", "Allows specifying an alternative ingredient registry file." },
			{ "--validate", "Checks if the target file can be loaded successfully, and contains valid data. Specifying this option will cause all other options to be ignored." },
			{ "--color <string_color>", "Change the color of ingredient names. String colors must include either an 'f' (foreground) or 'b' (background), then the name of the desired color." },
			{ "--ini <file>", "Allows specifying an alternative INI file. INI file is only loaded if it exists, otherwise default settings are used." },
			{ "--write_ini", "Writes a new INI file with default values." },
			}); ///< @brief Help documentation used by default.

			/**
			 * @function print_help(const Helper& = _default_doc)
			 * @brief Display help information.
			 * @param documentation	- Documentation to display
			 */
		void print(const Helper& documentation = _default_doc)
		{
			std::cout << documentation << std::endl;
		}
	}
}
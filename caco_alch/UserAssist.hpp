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
#include <utility>

namespace caco_alch {
	/**
	 * @function handle_arguments(opt::Param&&)
	 * @brief Handles program execution.
	 * @param args	- rvalue reference of an opt::Param instance.
	 * @param alch	- Alchemy instance rvalue.
	 * @param gs	- Gamesettings instance rvalue.
	 * @returns int - see main() documentation
	 */
	inline int handle_arguments(opt::Param&& args, Alchemy&& alch, GameSettings&& gs)
	{
		const Format _format{
			args.getFlag('q'),	// quiet
			args.getFlag('v'),	// verbose
			args.getFlag('e'),	// exact
			args.getFlag('a'),	// all
			args.getFlag('E'),	// export
			args.getFlag('R'),	// reverse
			3u,					// indent
			[&args]() { const auto v{ str::stoui(args.getv("precision")) }; if ( v != 0.0 ) return v; return 2u; }( ),
			[&args]() -> unsigned short { const auto v{ Color::strToColor(args.getv("color")) }; if ( v != 0 ) return v; return Color::_f_white; }( )
		};

		if ( args.getFlag('C') ) {
			std::stringstream buffer;
			buffer << std::cin.rdbuf();
			alch.print_build_to(std::cout, parseFileContent(buffer), std::forward<GameSettings>(gs), _format).flush();
		}
		else {
			if ( args.getFlag('l') ) // List mode
				alch.print_list_to(std::cout, _format).flush();
			if ( args.getFlag('b') ) // Build mode
				alch.print_build_to(std::cout, args._param, std::forward<GameSettings>(gs), _format).flush();
			else if ( args.getFlag('s') ) // Search mode
				for ( auto& it : args._param )
					alch.print_search_to(std::cout, it, _format).flush();
		}
		return 0;
	}

	inline int handle_arguments(std::tuple<opt::Param, Alchemy, GameSettings>&& pr) { return handle_arguments(std::forward<opt::Param>(std::get<0>(pr)), std::forward<Alchemy>(std::get<1>(pr)), std::forward<GameSettings>(std::get<2>(pr))); }

	// @brief Contains the list of valid commandline arguments for the alch program.
	inline opt::Matcher _matcher{ { 'l', 's', 'a', 'h', 'q', 'v', 'b', 'e', 'C', 'E' }, { { "load", true }, { "validate", false }, { "color", true }, { "precision", true }, { "name", true }, { "ini", true }, { "ini-alchemy-skill", true }, { "ini-alchemy-mod", true }, { "ini-reset", false } } };

	/**
	 * @namespace Help
	 * @brief Contains methods related to the inline terminal help display.
	 */
	namespace Help {
		/**
		 * @struct Helper
		 * @brief Provides a convenient extensible help display.
		 */
		struct Helper : ObjectBase {
		private:
			template<class T, typename = std::enable_if_t<std::is_same_v<T, std::string>>> struct more { bool operator()(const T& a, const T& b) const {
				if ( a.at(1) == '-' && b.at(1) != '-' )
					return false;
				else if ( a.at(1) != '-' && b.at(1) == '-' )
					return true;
				else return a < b;
			} };
		public:
			using Cont = std::map < std::string, std::string, more<std::string>> ;
			std::string _usage;
			Cont _doc;
			/**
			 * @constructor Helper(const std::string&, std::map<std::string, std::string>&&)
			 * @brief Default constructor, takes a string containing usage instructions, and a map of all commandline parameters and a short description of them.
			 * @param prog_name	- The string to show as the program name in the usage display. (ex. USAGE: <prog_name> <argument-syntax>)
			 * @param usage_str	- Brief string showing the commandline syntax for this program.
			 * @param doc		- A map where the key represents the commandline option, and the value is the documentation for that option.
			 */
			Helper(const std::string& prog_name, std::string usage_str, Cont doc) : ObjectBase(prog_name), _usage { std::move(usage_str) }, _doc{ std::move(doc) } { validate(); }

			/**
			 * @function validate()
			 * @brief Called at construction time, iterates through _doc and corrects any abnormalities.
			 */
			void validate()
			{
				for ( auto it{ _doc.begin() }; it != _doc.end(); ++it ) {
					auto& key{ it->first };
					const auto& doc{ it->second };
					if ( key.empty() ) // If key is empty, delete entry
						_doc.erase(it);
					else {                                                         // key isn't empty
						if ( const auto fst_char{ key.at(0) }; fst_char != '-' ) { // if key does not have a dash prefix

							_doc.erase(it);
							std::string mod{ '-' };
							if ( key.size() > 1 ) // is longopt
								mod += '-';
							_doc[mod + key] = doc;
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
				os << "Usage:\n  " << h._name << ' ' << h._usage << "\nOptions:\n";
				// get longest key name, use as indent
				std::streamsize indent{ 0 };
				for ( const auto& [key, doc] : h._doc )
					if ( const auto size{ key.size() }; size > indent )
						indent = static_cast<std::streamsize>(size);
				for ( auto& [key, doc] : h._doc )
					os << "  " << std::left << std::setw(indent + 2ll) << key << doc << '\n';
				os.flush();
				return os;
			}
		};

		const Helper _default_doc("caco-alch", "<[options] [target]>", {
			{ "-h", "Shows this help display." },
			{ "-l", "List all ingredients." },
			{ "-a", "Lists all ingredients and a list of all known effects." },
			{ "-s", "Searches the ingredient & effect lists for all additional parameters, and prints a result to STDOUT" },
			{ "-e", "Exact mode, does not allow partial search matches." },
			{ "-q", "Quiet output, only shows effects that match the search string in search results." },
			{ "-v", "Verbose output, shows extended stat information." },
			{ "-b", "(Incompatible with -s) Build mode, accepts up to 4 ingredient names and shows the result of combining them." },
			{ "-R", "(Not Implemented) Reverse order." }, // TODO
			{ "-C", "Receive an ingredient list from STDIN. (ex. \"cat <file> | caco-alch\")" },
			{ "-E", "File export mode, prints results in the format used by the parser so they can be read in again using '-C'." },
			{ "--load <file>", "Allows specifying an alternative ingredient registry file." },
			{ "--validate", "Checks if the target file can be loaded successfully, and contains valid data. Specifying this option will cause all other options to be ignored." },
			{ "--color <string_color>", "Change the color of ingredient names. String colors must include either an 'f' (foreground) or 'b' (background), then the name of the desired color." },
			{ "--precision <uint>", "Set the floating-point precision value when printing numbers. (Default: 2)" },
			{ "--ini-alchemy-skill <uint>", "Sets the alchemy skill level used when in build mode." },
			{ "--ini-alchemy-mod <uint>", "Sets the amount of the fortify alchemy effect added to the base skill." },
			{ "--ini <file>", "Load a specific INI file." },
			{ "--ini-reset", "Reset / Create an INI config file. (Used by the potion builder to calculate stats.)" },
			}); ///< @brief Help documentation used by default.

		/**
		 * @function print_help(const Helper& = _default_doc)
		 * @brief Display help information.
		 * @param documentation	- Documentation to display
		 */
		inline void print(const Helper& documentation = _default_doc)
		{
			std::cout << documentation << std::endl;
		}
	}
}
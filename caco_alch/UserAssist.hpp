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

#include "Alchemy.hpp"
#include "reloader.hpp"

namespace caco_alch {
	/**
	 * @function handle_arguments(opt::Param&&, Alchemy&&, GameSettings&&)
	 * @brief Handles primary program execution. Arguments can be retrieved from the init() function.
	 * @param args	- rvalue reference of an opt::Param instance.
	 * @param alch	- Alchemy instance rvalue.
	 * @param gs	- Gamesettings instance rvalue.
	 * @returns int - see main() documentation
	 */
	inline int handle_arguments(opt::Param&& args, Alchemy&& alch)
	{
		if ( args.getFlag('C') ) {
			std::stringstream buffer;
			buffer << std::cin.rdbuf();
			alch.print_build_to(std::cout, parseFileContent(buffer)).flush();
		}
		else {
			if ( args.getFlag('l') ) // List mode
				alch.print_list_to(std::cout).flush();
			if ( args.getFlag('b') ) // Build mode
				alch.print_build_to(std::cout, args._param).flush();
			else if ( const auto smart{ args.getFlag('S') }; args.getFlag('s') || smart ) { // Search mode
				if ( smart )
					alch.print_smart_search_to(std::cout, args._param);
				else
					for ( auto it{ args._param.begin() }; it != args._param.end(); ++it )
						alch.print_search_to(std::cout, *it).flush();
			}
		}
		if ( args.getFlag('S') ) {
			if ( const std::string filename{ "alch.cache" }; alch.writeCacheToFile(filename) ) {
			#ifdef ENABLE_DEBUG
				std::cout << sys::msg << "Successfully saved cache to \"" << filename << "\"" << std::endl;
			#endif
			}
			else {
			#ifdef ENABLE_DEBUG
				std::cout << sys::warn << "Failed to write cache to \"" << filename << "\"" << std::endl;
			#endif
			}
		}
		return 0;
	}

	inline int handle_arguments(std::tuple<opt::Param, Alchemy>&& pr) { return handle_arguments(std::forward<opt::Param>(std::get<0>(pr)), std::forward<Alchemy>(std::get<1>(pr))); }

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
			using Cont = std::map<std::string, std::string, more<std::string>>;
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
						_doc.erase(it, it);
					else {                                                         // key isn't empty
						if ( const auto fst_char{ key.at(0) }; fst_char != '-' ) { // if key does not have a dash prefix

							_doc.erase(it, it);
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

		/**
		 * @function print_help(const Helper& = _default_doc)
		 * @brief Display help information.
		 * @param documentation	- Documentation to display
		 */
		inline void print(const Helper& documentation)
		{
			std::cout << documentation << std::endl;
		}
	}

	/**
	 * @struct DefaultObjects
	 * @brief Contains the overridable default objects and values used in various parts of the program.
	 */
	struct DefaultObjects {
		// @brief Contains the list of valid commandline arguments for the alch program.
		opt::Matcher _matcher{  { 'l', 's', 'a', 'h', 'q', 'v', 'b', 'c', 'e', 'C', 'E', 'S' }, { { "load", true }, { "validate", false }, { "color", true }, { "precision", true }, { "name", true }, { "ini", true }, { "ini-load", true } }  };
		Help::Helper _help_doc{"caco-alch", "<[options] [target]>", {
			{ "-h", "Shows this help display." },
			{ "-l", "List all ingredients." },
			{ "-a", "Lists all ingredients and a list of all known effects." },
			{ "-s", "Searches the ingredient & effect lists for all additional parameters, and prints a result to STDOUT" },
			{ "-e", "Exact mode, does not allow partial search matches. This option is implicit when using build mode." },
			{ "-q", "Quiet output, only shows effects that match the search string in search results." },
			{ "-v", "Verbose output, shows extended stat information." },
			{ "-b", "(Incompatible with -s) Build mode, accepts up to 4 ingredient names and shows the result of combining them." },
			{ "-c", "Enable additional color support." },
			{ "-R", "Reverse sorting order." },
			{ "-C", "Receive an ingredient list from STDIN. (ex. \"cat <file> | alch -C\")" },
			{ "-E", "File export mode, prints results in the format used by the parser so they can be read in again using '-C'." },
			{ "-S", "Smart search, only searches for effects, but specifying multiple effects will only show ingredients that have at least 2 of them." },
			{ "--load <file>", "Allows specifying an alternative ingredient registry file." },
			{ "--validate", "Checks if the target file can be loaded successfully, and contains valid data. Specifying this option will cause all other options to be ignored." },
			{ "--color <string_color>", "Change the color of ingredient names. String colors must include either an 'f' (foreground) or 'b' (background), then the name of the desired color." },
			{ "--precision <uint>", "Set the floating-point precision value when printing numbers. (Default: 2)" },
			{ "--ini <<setting>:<value>>", "Change a value in the INI file. (Try \"cat <ini file>\" for variable names)" },
			{ "--ini-reset", "Reset / Create an INI config file. (Used by the potion builder to calculate stats.)" },
			{ "--ini-load <filepath>", "Load a specific INI file." },
		}};
		GameSettings::Cont _settings{
			{ "fAlchemyIngredientInitMult", 3.0 },
			{ "fAlchemySkillFactor", 3.0 },
			{ "fAlchemyAV", 15.0 },
			{ "fAlchemyMod", 0.0 },
			{ "fPerkAlchemyMasteryRank", 0.0 },		// valid: 0, 1, or 2
			{ "bPerkPoisoner", false },
			{ "bPerkAdvancedLab", false },
			{ "bPerkThatWhichDoesNotKillYou", false },
			{ "sPerkPhysicianType", std::string("") },
		};
	};
}

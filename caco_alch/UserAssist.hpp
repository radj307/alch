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
#include <Params.hpp>
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
	inline int handle_arguments(opt::Params&& args, Alchemy&& alch)
	{
		if ( args.check_flag('C') ) {
			std::stringstream buffer;
			buffer << std::cin.rdbuf();
			alch.print_build_to(std::cout, parseFileContent(buffer)).flush();
		}
		else {
			const auto params{ args.getAllWithType<opt::Parameter>() };
			if ( args.check_flag('l') ) // List mode
				alch.print_list_to(std::cout).flush();
			if ( args.check_flag('b') ) // Build mode
				alch.print_build_to(std::cout, params).flush();
			else if ( const auto smart{ args.check_flag('S') }; args.check_flag('s') || smart ) { // Search mode
				if ( smart )
					alch.print_smart_search_to(std::cout, params);
				else
					for ( auto it{ params.begin() }; it != params.end(); ++it )
						alch.print_search_to(std::cout, *it).flush();
			}
		}
		if ( args.check_flag('S') ) {
			if ( const std::string filename{ "alch.cache" }; alch.writeCacheToFile(filename) ) {
			#ifdef ENABLE_DEBUG
				std::cout << sys::term::msg << "Successfully saved cache to \"" << filename << "\"" << std::endl;
			#endif
			}
			else {
			#ifdef ENABLE_DEBUG
				std::cout << sys::term::warn << "Failed to write cache to \"" << filename << "\"" << std::endl;
			#endif
			}
		}
		return 0;
	}

	inline int handle_arguments(std::tuple<opt::Params, Alchemy>&& pr) { return handle_arguments(std::forward<opt::Params>(std::get<0>(pr)), std::forward<Alchemy>(std::get<1>(pr))); }

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

	struct DefaultPaths {
	private:
		static std::string combine(std::string left, std::string&& right)
		{
			return left.append(right);
		}
	public:
		DefaultPaths(std::string local_path, std::string config, std::string gamesettings, std::string registry) : _path_config{ combine(local_path, std::move(config)) }, _path_gamesettings{ combine(local_path, std::move(gamesettings)) }, _path_registry{ combine(local_path, std::move(registry)) } {}
		std::string
			_path_config,
			_path_gamesettings,
			_path_registry;
	};
	/**
	 * @struct DefaultObjects
	 * @brief Contains the overridable default objects and values used in various parts of the program.
	 */
	struct {
		const std::string _default_filename_config{ "alch.ini" }, _default_filename_gamesettings{ "alch.gamesettings" }, _default_filename_registry{ "alch.ingredients" };

		const std::string
			_help{ "help" },
			_load_config{ "load-ini" },
			_load_gamesettings{ "load-gamesettings" },
			_load_registry{ "load-registry" };
		const std::string
			_reset_gamesettings{ "reset-gamesettings" };
		const std::string
			_set_gamesetting{ "set" };

		const std::vector<std::string> _matcher{
			"color",
			"precision",
			"name",
			_set_gamesetting,
			_load_config,
			_load_gamesettings,
			_load_registry
		};

		// @brief Contains the list of valid commandline arguments for the alch program.
		/*const opt::Matcher _matcher{
			{ // FLAGS
				'l', // list
				's', // search
				'a', // all
				'h', // help
				'q', // quiet
				'v', // verbose
				'b', // build
				'c', // color
				'e', // exact
				'C', // import & build
				'E', // export
				'S'	 // advanced search
			},
			{ // OPTS
				{ _help, false },
				{ "validate", false },
				{ "color", true },
				{ "precision", true },
				{ "name", true },
				{ _set_gamesetting, true },
				{ _load_config, true },
				{ _load_gamesettings, true },
				{ _load_registry, true },
				{ _reset_gamesettings, false },
			}
		};*/
		const Help::Helper _help_doc{"caco-alch", "<[options] [target]>", {
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
			{ "-S", "Alternative search mode that takes any number of effects, (realistically limited to 4) and only displays ingredients that have at least 2 of them." },
			{ _help, "Shows this help display." },
			{ "--" + _load_registry + " <Registry-Path>", "Allows specifying an alternative ingredient registry file." },
			{ "--" + _load_config + " <INI-Path>", "Load a specific INI file." },
			{ "--validate", "Checks if the target file can be loaded successfully, and contains valid data. Specifying this option will cause all other options to be ignored." },
			{ "--color <string_color>", "Change the color of ingredient names. String colors must include either an 'f' (foreground) or 'b' (background), then the name of the desired color." },
			{ "--precision <uint>", "Set the floating-point precision value when printing numbers. (Default: 2)" },
			{ "--" + _set_gamesetting + " << setting>:<value >>", "Change a value in the Game Settings file. ( Try \"cat <ini file>\" for variable names)"},
			{ "--" + _reset_gamesettings, "Reset / Create a Game Settings config file. (Used by the potion builder to calculate stats.)"},
			{ "--" + _load_gamesettings + " <GameSettings-Path>", "Load a specific Game Settings file."},
		}};
		const GameSettings::Cont _settings{
			{ "fAlchemyIngredientInitMult", 3.0 },
			{ "fAlchemySkillFactor", 3.0 },
			{ "fAlchemyAV", 15.0 },
			{ "fAlchemyMod", 0.0 },
			{ "fPerkAlchemyMasteryRank", 0.0 },		// valid: 0, 1, or 2
			{ "fPerkPoisonerFactor", 0.05 },
			{ "bPerkPoisoner", false },
			{ "bPerkPhysician", false },
			{ "bPerkPureMixture", false },
			{ "bPerkBenefactor", false },
			{ "bPerkAdvancedLab", false },
			{ "bPerkThatWhichDoesNotKillYou", false },
			{ "sPerkPhysicianType", std::string("") },
		};
	} DefaultObjects;
}

#pragma once
#include <file.h>
#include <resolve-path.hpp>

#include "Alchemy.hpp"
#include "GameSettings.hpp"
#include "reloader.hpp"
#include "UserAssist.hpp"

namespace caco_alch {

	/**
	 * @function handle_ini(const std::string&, const opt::Param&, const DefaultObjects&)
	 * @brief Handle INI-related initialization for the init() function.
	 * @param ini_filename		- INI config filepath. If it doesn't exist, default settings are returned.
	 * @param args				- Commandline arguments.
	 * @param def				- Default settings to use
	 * @returns GameSettings	- Contains the game settings instance parsed from the INI file, and all of the default settings not overridden by it.
	 *\n						  If a value is missing from the INI, but the file does exist, the default value from def is passed so it isn't null.
	 */
	inline GameSettings handle_ini(const std::string& ini_filename, const opt::Param& args, const GameSettings::Cont& def)
	{
		const auto ini_exists{ file::exists(ini_filename) };

		GameSettings gs(def); // INIT
		// Load current INI settings if the file exists
		if ( ini_exists ) gs.read_ini(ini_filename);
		if ( ini_exists || args.check_opt("ini") || args.check_opt("ini-reset") ) {
			bool update_ini_before_return{ false }; ///< @brief Changed by the set() lambda, updates the ini with new values before returning when true.

			// Lambda that writes GameSettings to INI
			const auto write_ini{ [&ini_filename, &gs](){ return file::write(ini_filename, gs.to_stream(), false); } };

			// check if the --ini-reset option was specified, and write to INI if it was
			if ( args.check_opt("ini-reset") && write_ini() )
				std::cout << sys::msg << "Successfully reset INI. ( " << ini_filename << " )\n";

			// check for INI option
			if (args.check_opt("ini")) {
				// lambda used to set INI variables
				const auto set{ [&gs, &update_ini_before_return](const std::string& name, const std::string& value) {
					try {
						if (gs.set(name, value)) {
							std::cout << sys::msg << "\'" << name << "\' = \'" << value << "\'\n";
							update_ini_before_return = true;
						}
						else std::cout << sys::warn << "Operation failed without exception.\n";
					} catch (std::exception& ex) {
						std::cout << sys::error << "\'" << name <<  "\' = \'" << value << "\' triggered an exception: \"" << ex.what() << '\"' << std::endl;
					}
				} };
				// iterate through all --ini args
				for ( auto i{ args.get_index("ini") }; i != args._opt.size(); i = args.get_index("ini", i + 1) )
					if ( const auto arg{ args.getv("ini", i) }; !arg.empty() ) {
						if (const auto dPos{ arg.find(':') }; str::pos_valid(dPos) && !str::pos_valid(arg.find(':', dPos + 1)))
							set(arg.substr(0u, dPos), arg.substr(dPos + 1u));
						else std::cout << sys::warn << "Invalid Parameter for --ini: Couldn't find ':' in \"" << arg << "\"" << std::endl;
					}
			}
			// check if the INI should be updated
			if ( update_ini_before_return ) {
				if ( write_ini() )
					std::cout << sys::msg << "Successfully wrote to \"" << ini_filename << '\"' << std::endl;
				else std::cout << sys::warn << "Failed to write to \"" << ini_filename << '\"' << std::endl;
			}
		}
		return gs; // RETURN
	}

	/**
	 * @function init(const int, char*[], char*[])
	 * @brief Initialize the program and its assets, process some commandline arguments, and parse configs.
	 * @param argc	- See main()
	 * @param argv	- See main()
	 * @param envp	- See main()
	 * @returns std::tuple<opt::Param, Alchemy, GameSettings>
	 *\n		0	- Parsed commandline arguments.
	 *\n		1	- Alchemy instance containing the parsed ingredient registry.
	 *\n		2	- Parsed INI config including all default settings.
	 */
	inline std::tuple<opt::Param, Alchemy> init(const int argc, char* argv[], char* envp[])
	{
		const DefaultObjects def;
		// parse arguments
		opt::Param args(argc, argv, def._matcher);
	#ifndef ENABLE_DEBUG
		if ( args.empty() ) { // print help if no valid parameters found.
			print(def._help_doc);
			throw std::exception("No valid parameters detected.");
		}
	#endif
		if ( args.getFlag('h') )
			print(def._help_doc);

		const auto path{ opt::resolve_split_path(envp, argv[0]).first };

		std::string filename{ path }, ini_filename{ path };
		if (args.check_opt("load")) {
			filename.append(args.getv("load"));
			std::cout << sys::msg << "Changed target registry path to \"" << filename << "\"" << std::endl;
		}
		else filename.append("caco-ingredient-list.dat");

		if (args.check_opt("ini-load")) {
			ini_filename.append(args.getv("ini-load"));
			std::cout << sys::msg << "Changed target INI path to \"" << ini_filename << "\"" << std::endl;
		}
		else ini_filename.append("alch.ini");

		if ( args.check_opt("validate") ) { // Process "--validate" opt
			constexpr auto indent_max{ 16u };
			std::cout << "argv[0]" << std::setw(indent_max - 7u) << ' ' << argv[0] << std::endl;
			std::cout << "location" << std::setw(indent_max - 8u) << ' ' << path << std::endl;
			std::cout << "registry" << std::setw(indent_max - 8u) << ' ' << filename << std::endl;
			std::cout << "GMST config" << std::setw(indent_max - 11u) << ' ' << ini_filename << std::endl;
			if ( validate_file(filename) )
				std::cout << sys::msg << "Validation succeeded." << std::endl;
			else std::cout << sys::warn << "Validation failed." << std::endl;
		}

		const Format _format{
			args.getFlag('q'),	// quiet
			args.getFlag('v'),	// verbose
			args.getFlag('e'),	// exact
			args.getFlag('a'),	// all
			args.getFlag('E'),	// export
			args.getFlag('R'),	// reverse
			args.getFlag('c'),	// allow color
			args.getFlag('S'),	// smart search
			3u,					// indent
			[&args]() { const auto v{ str::stoui(args.getv("precision")) }; if ( v != 0.0 ) return v; return 2u; }( ),
			[&args]() -> short { const auto v{ Color::strToColor(args.getv("color")) }; if ( v != 0 ) return v; return Color::_white; }( )
		};

		return { args, Alchemy(loadFromFile(filename), _format, handle_ini(ini_filename, args, def._settings)) };
	}
}

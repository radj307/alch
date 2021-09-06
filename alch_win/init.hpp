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
	 * @brief Handle Game Setting-related initialization for the init() function.
	 * @param filename			- Game Setting config filepath. If it doesn't exist, default settings are returned.
	 * @param args				- Commandline arguments.
	 * @param def				- Default settings to use
	 * @returns GameSettings	- Contains the game settings instance parsed from the INI file, and all of the default settings not overridden by it.
	 *\n						  If a value is missing from the Game Settings, but the file does exist, the default value from def is passed so it isn't null.
	 */
	inline GameSettings handleGameSettings(const std::string& filename, const opt::Param& args, const GameSettings::Cont& def)
	{
		const auto exists{ file::exists(filename) };

		GameSettings gs(def); // INIT
		// Load current INI settings if the file exists
		if ( exists ) gs.read_ini(filename);
		if ( exists || args.check_opt(DefaultObjects._set_gamesetting) || args.check_opt(DefaultObjects._reset_gamesettings) ) {
			bool update_ini_before_return{ false }; ///< @brief Changed by the set() lambda, updates the ini with new values before returning when true.

			// Lambda that writes GameSettings to INI
			const auto write_ini{ [&filename, &gs](){ return file::write(filename, gs.to_stream(), false); } };

			// check if the --ini-reset option was specified, and write to INI if it was
			if ( args.check_opt(DefaultObjects._reset_gamesettings) && write_ini() )
				std::cout << sys::msg << "Successfully reset GMST Config. ( " << filename << " )\n";

			// check for INI option
			if (args.check_opt(DefaultObjects._set_gamesetting)) {
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
				for ( auto i{ args.get_index(DefaultObjects._set_gamesetting) }; i != args._opt.size(); i = args.get_index(DefaultObjects._set_gamesetting, i + 1) )
					if ( const auto arg{ args.getv(DefaultObjects._set_gamesetting, i) }; !arg.empty() ) {
						if (const auto dPos{ arg.find(':') }; str::pos_valid(dPos) && !str::pos_valid(arg.find(':', dPos + 1)))
							set(arg.substr(0u, dPos), arg.substr(dPos + 1u));
						else std::cout << sys::warn << "Invalid Parameter for --ini: Couldn't find ':' in \"" << arg << "\"" << std::endl;
					}
			}
			// check if the INI should be updated
			if ( update_ini_before_return ) {
				if ( write_ini() )
					std::cout << sys::msg << "Successfully wrote to \"" << filename << '\"' << std::endl;
				else std::cout << sys::warn << "Failed to write to \"" << filename << '\"' << std::endl;
			}
		}
		return gs; // RETURN
	}

	inline file::xINI handleINI(const std::string& filename, const opt::Param& args, const std::string& def_filename)
	{
		if ( file::exists(filename) )
			return file::xINI{ filename };
		else if ( filename != def_filename && file::exists(def_filename) )
			return file::xINI{ def_filename };
		else return {};
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
		// parse arguments
		opt::Param args(argc, argv, DefaultObjects._matcher);
	#ifndef ENABLE_DEBUG
		if ( args.empty() ) { // print help if no valid parameters found.
			print(DefaultObjects._help_doc);
			throw std::exception("No valid parameters detected.");
		}
	#endif
		if ( args.getFlag('h') || args.check_opt(DefaultObjects._help) )
			print(DefaultObjects._help_doc);

		const auto local_path{ opt::resolve_split_path(envp, argv[0]).first };

		const auto getOptOrDefault{ [&args](const std::string& optname, const std::string& defv){
			if ( args.check_opt(optname) )
				return args.getv(optname);
			return defv;
		} };

		DefaultPaths paths(
			local_path, 
			getOptOrDefault(DefaultObjects._load_config, DefaultObjects._default_filename_config),
			getOptOrDefault(DefaultObjects._load_gamesettings, DefaultObjects._default_filename_gamesettings),
			getOptOrDefault(DefaultObjects._load_registry, DefaultObjects._default_filename_registry)
		);

		auto ini{ handleINI(paths._path_config, args, DefaultObjects._default_filename_config) };

		// add ini functions here
		if ( const auto regPath{ ini.getv("registry", "path") }; !regPath.empty() && !args.check_opt(DefaultObjects._load_registry) )
			paths._path_registry = regPath;

		if ( const auto GMSTPath{ ini.getv("GMST", "path") }; !GMSTPath.empty() && !args.check_opt(DefaultObjects._load_gamesettings) )
			paths._path_gamesettings = GMSTPath;

		auto gmst{ handleGameSettings(paths._path_gamesettings, args, DefaultObjects._settings) };

		if ( args.check_opt("validate") ) { // Process "--validate" opt
			constexpr auto indent_max{ 16u };
			std::cout << "argv[0]" << std::setw(indent_max - 7u) << ' ' << argv[0] << std::endl;
			std::cout << "directory" << std::setw(indent_max - 9u) << ' ' << local_path << std::endl;
			std::cout << "registry" << std::setw(indent_max - 8u) << ' ' << paths._path_registry << std::endl;
			std::cout << "INI Config" << std::setw(indent_max - 10u) << ' ' << paths._path_config << std::endl;
			std::cout << "GMST Config" << std::setw(indent_max - 11u) << ' ' << paths._path_gamesettings << std::endl;

			if ( validate_file(paths._path_registry) )
				std::cout << sys::msg << "Registry Validation Succeeded." << std::endl;
			else std::cout << sys::warn << "Failed to find Registry!" << std::endl;

			if ( file::exists(paths._path_config) )
				std::cout << sys::msg << "INI Config Exists." << std::endl;
			else std::cout << sys::warn << "Failed to find INI Config!" << std::endl;

			if ( file::exists(paths._path_gamesettings) )
				std::cout << sys::msg << "GMST Config Exists." << std::endl;
			else std::cout << sys::warn << "Failed to find GMST Config!" << std::endl;
		}

		Format _format{
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
			[&args]() -> short { const auto v{ color::strToColor(args.getv("color")) }; if ( v != 0 ) return v; return color::white; }( )
		};

		return { args, Alchemy(loadFromFile(paths._path_registry), _format, gmst) };
	}
}

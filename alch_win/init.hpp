#pragma once
#include <file.h>
#include <resolve-path.hpp>
#include <StrToColor.hpp>
#include <INI.hpp>

#include <Alchemy.hpp>
#include <GameConfig.hpp>
#include <reloader.hpp>
#include <handle_arguments.hpp>
#include <ColorConfigLoader.hpp>


namespace caco_alch {

	/**
	 * @function handle_ini(const std::string&, const opt::Param&, const DefaultObjects&)
	 * @brief Handle Game Setting-related initialization for the init() function.
	 * @param filename			- Game Setting config filepath. If it doesn't exist, default settings are returned.
	 * @param args				- Commandline arguments.
	 * @param def				- Default settings to use
	 * @returns GameConfig	- Contains the game settings instance parsed from the INI file, and all of the default settings not overridden by it.
	 *\n						  If a value is missing from the Game Settings, but the file does exist, the default value from def is passed so it isn't null.
	 */
	inline GameConfig handleGameSettings(const std::string& filename, const opt::Params& args, const GameConfig::Cont& def)
	{
		const auto exists{ file::exists(filename) };

		GameConfig gs(def); // INIT
		// Load current INI settings if the file exists
		if ( exists ) gs.read_ini(filename);
		if ( exists || args.check_opt(DefaultObjects._set_gamesetting) || args.check_opt(DefaultObjects._reset_gamesettings) ) {
			bool update_ini_before_return{ false }; ///< @brief Changed by the set() lambda, updates the ini with new values before returning when true.

			// Lambda that writes GameConfig to INI
			const auto write_ini{ [&filename, &gs](){ return file::write(filename, gs.to_stream(), false); } };

			// check if the --ini-reset option was specified, and write to INI if it was
			if ( args.check_opt(DefaultObjects._reset_gamesettings) && write_ini() )
				std::cout << sys::term::msg << "Successfully reset GMST Config. ( " << filename << " )\n";

			// check for INI option
			if (args.check_opt(DefaultObjects._set_gamesetting)) {
				// lambda used to set INI variables
				const auto set{ [&gs, &update_ini_before_return](const std::string& name, const std::string& value) {
					try {
						if (gs.set(name, value)) {
							std::cout << sys::term::msg << "\'" << name << "\' = \'" << value << "\'\n";
							update_ini_before_return = true;
						}
						else std::cout << sys::term::warn << "Operation failed without exception.\n";
					} catch (std::exception& ex) {
						std::cout << sys::term::error << "\'" << name <<  "\' = \'" << value << "\' triggered an exception: \"" << ex.what() << '\"' << std::endl;
					}
				} };

				// iterate through all --ini args
				for ( auto it{ args.find(DefaultObjects._set_gamesetting) }; it != args.end(); it = args.find(DefaultObjects._set_gamesetting, it + 1u))
					if (const auto arg{ it->getv() }; arg.has_value() && !arg.value().empty()) {
						if (const auto dPos{ arg.value().find(':') }; str::pos_valid(dPos) && !str::pos_valid(arg.value().find(':', dPos + 1)))
							set(arg.value().substr(0u, dPos), arg.value().substr(dPos + 1u));
						else std::cout << sys::term::warn << "Invalid Parameter for --ini: Couldn't find ':' in \"" << arg.value() << "\"" << std::endl;
					}
			}
			// check if the INI should be updated
			if ( update_ini_before_return ) {
				if ( write_ini() )
					std::cout << sys::term::msg << "Successfully wrote to \"" << filename << '\"' << std::endl;
				else std::cout << sys::term::warn << "Failed to write to \"" << filename << '\"' << std::endl;
			}
		}
		return gs; // RETURN

	}

	/**
	 * @brief Print whether critical files were successfully located.
	 * @param args			- Args ref
	 * @param paths			- Paths ref
	 * @param argv0			- argv[0]
	 * @param local_path	- The path to the directory where the program is located
	 * @param indent_max	- Max indentation, used to align output.
	 */
	inline void validate(const opt::Params& args, const DefaultPaths& paths, const std::string& argv0, const std::string& local_path, const std::streamsize indent_max)
	{
		std::cout << "argv[0]" << std::setw(indent_max - 7u) << ' ' << argv0 << std::endl;
		std::cout << "directory" << std::setw(indent_max - 9u) << ' ' << local_path << std::endl;
		std::cout << "registry" << std::setw(indent_max - 8u) << ' ' << (file::exists(paths._path_registry) ? color::f::green : color::f::red) << paths._path_registry << color::reset << std::endl;
		std::cout << "INI Config" << std::setw(indent_max - 10u) << ' ' << (file::exists(paths._path_config) ? color::f::green : color::f::red) << paths._path_config << color::reset << std::endl;
		std::cout << "GMST Config" << std::setw(indent_max - 11u) << ' ' << (file::exists(paths._path_gamesettings) ? color::f::green : color::f::red) << paths._path_gamesettings << color::reset << std::endl;
	}

	inline std::tuple<opt::Params, Alchemy> init_noConfig(opt::Params& args, DefaultPaths& paths)
	{
		auto gmst{ handleGameSettings(paths._path_gamesettings, args, DefaultObjects._settings) };

		Format _format{
			args.check_flag('q'),	// quiet
			args.check_flag('v'),	// verbose
			args.check_flag('e'),	// exact
			args.check_flag('a'),	// all
			args.check_flag('E'),	// export
			args.check_flag('R'),	// reverse
			args.check_flag('c'),	// allow color
			args.check_flag('S'),	// smart search
			[&args]() { const auto val{ args.getv("precision") }; return val.has_value() && !val.value().empty() ? str::stoui(val.value()) : 2u; }(),
		};

		return { args, Alchemy(loadFromFile(paths._path_registry), _format, gmst) };
	}

	inline std::tuple<opt::Params, Alchemy> init_config(opt::Params& args, DefaultPaths& paths, const std::string& iniPath)
	{
		file::ini::INI ini{ };

		// add ini functions here
		if (const auto regPath{ ini.getv("paths", "registry") }; !regPath.empty() && !args.check_opt(DefaultObjects._load_registry))
			paths._path_registry = regPath;

		if (const auto GMSTPath{ ini.getv("paths", "gameconfig") }; !GMSTPath.empty() && !args.check_opt(DefaultObjects._load_gamesettings))
			paths._path_gamesettings = GMSTPath;

		auto gmst{ handleGameSettings(paths._path_gamesettings, args, DefaultObjects._settings) };

		Format _format{
			ini,
			args.check_flag('q'),	// quiet
			args.check_flag('v'),	// verbose
			args.check_flag('e'),	// exact
			args.check_flag('a'),	// all
			args.check_flag('E'),	// export
			args.check_flag('R'),	// reverse
			args.check_flag('c'),	// allow color
			args.check_flag('S'),	// smart search
			[&args]() { const auto val{ args.getv("precision") }; return val.has_value() && !val.value().empty() ? str::stoui(val.value()) : 2u; }(),
		};

		return { args, Alchemy(loadFromFile(paths._path_registry), _format, gmst) };
	}

	/**
	 * @function init(const int, char*[], char*[])
	 * @brief Initialize the program and its assets, process some commandline arguments, and parse configs.
	 * @param argc	- See main()
	 * @param argv	- See main()
	 * @param envp	- See main()
	 * @returns std::tuple<opt::Param, Alchemy, GameConfig>
	 *\n		0	- Parsed commandline arguments.
	 *\n		1	- Alchemy instance containing the parsed ingredient registry.
	 *\n		2	- Parsed INI config including all default settings.
	 */
	inline std::tuple<opt::Params, Alchemy> init(opt::Params& args, DefaultPaths& paths)
	{
	#ifndef ENABLE_DEBUG
		if ( args.empty() ) { // print help if no valid parameters found.
			std::cout << DefaultObjects._help_doc;
			//Help::print(DefaultObjects._help_doc);
			throw std::exception("No valid parameters detected.");
		}
	#endif
		if (args.check_flag('h') || args.check_opt(DefaultObjects._help))
			std::cout << DefaultObjects._help_doc;
			//Help::print(DefaultObjects._help_doc);

		if (const auto precision{ args.getv("precision") }; precision.has_value()) {
			std::cout.precision(str::stoll(precision.value()));
		}

		if (const auto iniPath{ (args.check(DefaultObjects._load_config) ? args.getv(DefaultObjects._load_config).value_or(paths._path_config) : paths._path_config) }; file::exists(iniPath))
			return init_config(args, paths, iniPath);
		return init_noConfig(args, paths);
	}
}

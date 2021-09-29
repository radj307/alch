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
	inline GameConfig loadGameConfig(const std::string& filename, const opt::Params& args, const GameConfig::Cont& defaults)
	{
		const auto exists{ file::exists(filename) };

		GameConfig gs(defaults); // INIT
		// Load current INI settings if the file exists
		if (exists) gs.read_ini(filename);
		if (exists || args.check_opt(DefaultObjects._set_gamesetting) || args.check_opt(DefaultObjects._reset_gamesettings)) {
			bool update_ini_before_return{ false }; ///< @brief Changed by the set() lambda, updates the ini with new values before returning when true.

			// Lambda that writes GameConfig to INI
			const auto write_ini{ [&filename, &gs]() { return file::write(filename, gs.to_stream(), false); } };

			// check if the --ini-reset option was specified, and write to INI if it was
			if (args.check_opt(DefaultObjects._reset_gamesettings) && write_ini())
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
						std::cout << sys::term::error << "\'" << name << "\' = \'" << value << "\' triggered an exception: \"" << ex.what() << '\"' << std::endl;
					}
				} };

				// iterate through all --ini args
				for (auto it{ args.find(DefaultObjects._set_gamesetting) }; it != args.end(); it = args.find(DefaultObjects._set_gamesetting, it + 1u))
					if (const auto arg{ it->getv() }; arg.has_value() && !arg.value().empty()) {
						if (const auto dPos{ arg.value().find(':') }; str::pos_valid(dPos) && !str::pos_valid(arg.value().find(':', dPos + 1)))
							set(arg.value().substr(0u, dPos), arg.value().substr(dPos + 1u));
						else std::cout << sys::term::warn << "Invalid Parameter for --ini: Couldn't find ':' in \"" << arg.value() << "\"" << std::endl;
					}
			}
			// check if the INI should be updated
			if (update_ini_before_return) {
				if (write_ini())
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

	struct Instance {
		using ConfigType = std::optional<file::ini::INI>;

		opt::Params Arguments;
		DefaultPaths Paths;
		ConfigType Config;
		Alchemy Alchemy;

		Instance(opt::Params args, DefaultPaths paths) :
			Arguments{ std::move(args) },
			Paths{ std::move(paths) },
			Config{ [this]() -> ConfigType {
				const auto iniPath{ (Arguments.check(DefaultObjects._load_config) ? Arguments.getv(DefaultObjects._load_config).value_or(Paths._path_config) : Paths._path_config) };
				return file::exists(iniPath) ? file::ini::INI(iniPath) : static_cast<ConfigType>(std::nullopt);
			}() },
			Alchemy{ loadFromFile(Paths._path_registry), { Arguments, Config }, loadGameConfig(Paths._path_gamesettings, Arguments, DefaultObjects._settings) } {}


		const static int RETURN_SUCCESS{ 0 };
		const static int RETURN_FAILURE{ 1 };

		int handleArguments(std::ostream& os) const
		{
			if (Arguments.check_flag('i')) {
				std::stringstream buffer;
				buffer << std::cin.rdbuf();
				Alchemy.print_build(os, parseFileContent(buffer)).flush();
				return RETURN_SUCCESS;
			}
			else if (Arguments.check_flag('l')) {
				Alchemy.print_list(os).flush();
				return RETURN_SUCCESS;
			}
			else {
				const auto params{ Arguments.getAllWithType<opt::Parameter>() };
				if (Arguments.check_flag('b')) {
					Alchemy.print_build(os, params).flush();
					return RETURN_SUCCESS;
				}

				else if (Arguments.check_flag('S')) {
					Alchemy.print_smart_search(os, params).flush();
					return RETURN_SUCCESS;
				}

				else if (Arguments.check_flag('s')) {
					for (auto& arg : params)
						Alchemy.print_search(os, arg).flush();
					return RETURN_SUCCESS;
				}
			}
			return RETURN_FAILURE;
		}
		int handleArguments() const
		{
			return handleArguments(std::cout);
		}
	};
}
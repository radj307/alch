#pragma once
#include <fileio.hpp>
//#include <resolve-path.hpp>
#include <StrToColor.hpp>
#include <INI.hpp>

#include <Alchemy.hpp>
#include <GameConfig.hpp>
#include <reloader.hpp>
#include <handle_arguments.hpp>
#include <DefaultObjects.hpp>
//#include <ColorConfigLoader.hpp>


namespace caco_alch {
	inline GameConfig loadGameConfig(const std::string& filename, const opt::ParamsAPI2& args, const GameConfig::Cont& defaults)
	{
		GameConfig gs(defaults); // INIT
		// check for the reset INI option
		if (args.check<opt::Option>(DefaultObjects._reset_gamesettings)) {
			if (file::write(filename, gs.to_stream(), false))
				std::cout << sys::term::msg << "Successfully reset Game Config \"" << filename << "\"\n";
			else
				std::cout << sys::term::error << "Failed to reset Game Config \"" << filename << "\" (Check write permissions)\n";
		}
		bool update_ini_before_return{ false };
		const auto set{ [&gs, &update_ini_before_return](const std::string& name, const std::string& value) {
			try {
				if (gs.set(name, value)) {
					std::cout << sys::term::msg << "\'" << name << "\' = \'" << value << "\'\n";
					update_ini_before_return = true;
				}
				else
					std::cout << sys::term::warn << "Set operation failed without exception.\n";
			} catch (std::exception& ex) {
				std::cout << sys::term::error << "Setting \"" << name << "\" to \"" << value << "\" caused an exception: \"" << ex.what() << '\"' << std::endl;
			}
		} };
		// iterate through all --set arguments
		for (auto& it : args.typeget_all<opt::Option>(DefaultObjects._set_gamesetting)) {
			if (it.hasv()) {
				const auto& [name, val] {str::split(it.getv().value(), ':')};
				set(name, val);
			}
			else throw std::exception("Invalid usage of \"--set\" command!");
		}
		/*for (auto it{args.find(DefaultObjects._set_gamesetting)}; it != args.end(); it = args.find(DefaultObjects._set_gamesetting, it + 1)) {
			if (const auto arg{}; arg.has_value() && !arg.value().empty()) {
				if (const auto pos{ arg.value().find(':') }; str::pos_valid(pos) && !str::pos_valid(arg.value().find(':', pos + 1)))
					set(arg.value().substr(0u, pos), arg.value().substr(pos + 1));
				else
					throw std::exception(std::string("Invalid \"--set\" command: \"" + arg.value() + "\"").c_str());
			}
		}*/
		// if the INI configuration has changed, write it to file
		if (update_ini_before_return) {
			if (file::write(filename, gs.to_stream(), false))
				std::cout << sys::term::msg << "Successfully wrote to \"" << filename << '\"' << std::endl;
			else
				std::cout << sys::term::warn << "Failed to write to \"" << filename << '\"' << std::endl;
		}
		return gs; // RETURN
	}
	/**
	 * @struct Instance
	 * @brief Represents an instance of the caco_alch program. Contains all the high-level methods used to run the program.
	 */
	struct Instance {
		using ConfigType = std::optional<file::ini::INI>;

		opt::ParamsAPI2 Arguments;
		DefaultPaths Paths;
		ConfigType Config;
		Alchemy Alchemy;

		Instance(opt::ParamsAPI2 args, DefaultPaths paths) :
			Arguments{ std::move(args) },
			Paths{ std::move(paths) },
			Config{ [this]() -> ConfigType {
			const auto iniPath{ (Arguments.check<opt::Option>(DefaultObjects._load_config) ? Arguments.typegetv<opt::Option>(DefaultObjects._load_config).value_or(Paths._path_config) : Paths._path_config) };
			return file::exists(iniPath) ? file::ini::INI(iniPath) : static_cast<ConfigType>(std::nullopt);
		}() },
			Alchemy{ loadFromFile(Paths._path_registry), { Arguments, Config }, loadGameConfig(Paths._path_gamesettings, Arguments, DefaultObjects._settings) } {}


		const static int RETURN_SUCCESS{ 0 };
		const static int RETURN_FAILURE{ 1 };

		/**
		 * @brief Validate the location of the configuration files used by the program.
		 * @param os	 - Target output stream.
		 * @param indent - Maximum indentation value, from which the length of all prefixes are subtracted to create uniform aligned values.
		 */
		void validate(std::ostream& os, const std::streamsize indent = 20ll) const
		{
			os << "argv[0]" << std::setw(indent - 7) << ' ' << (file::exists(Arguments.arg0()) ? color::f::green : color::f::red) << Arguments.arg0().value_or("") << '\n';
			os << "directory" << std::setw(indent - 9) << ' ' << (file::exists(Paths._local) ? color::f::green : color::f::red) << Paths._local << '\n';
			os << "registry" << std::setw(indent - 8) << ' ' << (file::exists(Paths._path_registry) ? color::f::green : color::f::red) << Paths._path_registry << color::reset << '\n';
			os << "INI Config" << std::setw(indent - 10) << ' ' << (file::exists(Paths._path_config) ? color::f::green : color::f::red) << Paths._path_config << color::reset << '\n';
			os << "Game Config" << std::setw(indent - 11) << ' ' << (file::exists(Paths._path_gamesettings) ? color::f::green : color::f::red) << Paths._path_gamesettings << color::reset << '\n';
		}
		/**
		 * @brief Validate the location of the configuration files used by the program and print it to std::cout.
		 */
		void validate() const { validate(std::cout); }
		/**
		 * @brief Handle the received commandline arguments and call the relevant functions with the remaining parameters.
		 * @param os	- Target output stream.
		 * @returns int
		 */
		int handleArguments(std::ostream& os) const
		{
			if (Arguments.check<opt::Flag>('i')) {
				std::stringstream buffer;
				buffer << std::cin.rdbuf();
				Alchemy.print_build(os, parseFileContent(buffer)).flush();
				return RETURN_SUCCESS;
			}
			else if (Arguments.check<opt::Flag>('l')) {
				Alchemy.print_list(os).flush();
				return RETURN_SUCCESS;
			}
			else {
				const auto params{ [this]() -> std::vector<std::string> {
					const auto par{ Arguments.typeget_all<opt::Parameter>() };
					std::vector<std::string> vec;
					vec.reserve(par.size());
					for (auto& it : par)
						vec.emplace_back(it.name());
					vec.shrink_to_fit();
					return vec;
				}() };
				if (Arguments.check<opt::Flag>('b')) {
					Alchemy.print_build(os, params, 4u).flush();
					return RETURN_SUCCESS;
				}

				else if (Arguments.check<opt::Flag>('S')) {
					Alchemy.print_smart_search(os, params).flush();
					return RETURN_SUCCESS;
				}

				else if (Arguments.check<opt::Flag>('s')) {
					for (auto& arg : params)
						Alchemy.print_search(os, arg).flush();
					return RETURN_SUCCESS;
				}
			}
			return RETURN_FAILURE;
		}
		/**
		 * @brief Handle the received commandline arguments and call the relevant functions with the remaining parameters.
		 * @returns int
		 */
		int handleArguments() const
		{
			return handleArguments(std::cout);
		}
	};
}
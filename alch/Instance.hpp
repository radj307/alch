#pragma once
#include <fileio.hpp>
#include <fileutil.hpp>
#include <color-transform.hpp>
#include <INI.hpp>

#include <env.hpp>
#include <envpath.hpp>
#include <str.hpp>
#include <opt3.hpp>

#include <Alchemy.hpp>
#include <GameConfig.hpp>
#include <reloader.hpp>
#include <handle_arguments.hpp>
#include <DefaultObjects.hpp>
//#include <ColorConfigLoader.hpp>


namespace caco_alch {
	/**
	 * @brief
	 * @param filename
	 * @param args
	 * @param defaults	Default GameConfig Values
	 * @returns			GameConfig
	 */
	inline GameConfig loadGameConfig(const std::filesystem::path& filename, const opt3::ArgManager& args, const GameConfig::Cont& defaults)
	{
		GameConfig gs{ defaults };

		// check for the reset INI option
		if (args.check<opt3::Option>("reset-gamesettings")) {
			if (file::write_to(filename, gs.to_stream(), false))
				std::cout << term::msg << "Successfully reset Game Config \"" << filename << "\"\n";
			else
				std::cout << term::error << "Failed to reset Game Config \"" << filename << "\" (Check write permissions)\n";
		}
		bool update_ini_before_return{ false };
		bool modified{ false };
		const auto set{ [&gs, &modified](const std::string& name, const std::string& value) {
			try {
				if (gs.set(name, value)) {
					std::cout << term::msg << name << " = " << value << "\n";
					//update_ini_before_return = true;
					modified = true;
				}
				else std::cout << term::warn << "Couldn't set the value of '" << name << "'; does that key exist?" << std::endl;
			} catch (std::exception& ex) {
				std::cout << term::error << "Setting \"" << name << "\" to \"" << value << "\" caused an exception: \"" << ex.what() << '\"' << std::endl;
			}
		} };
		// iterate through all --set arguments
		for (auto& it : args.get_all<opt3::Option>("set")) {
			if (it.has_value()) {
				const auto& [name, val] {str::split(it.value(), ':')};
				set(name, val);
			}
		}
		/*for (auto it{args.find("set")}; it != args.end(); it = args.find("set", it + 1)) {
			if (const auto arg{}; arg.has_value() && !arg.value().empty()) {
				if (const auto pos{ arg.value().find(':') }; str::pos_valid(pos) && !str::pos_valid(arg.value().find(':', pos + 1)))
					set(arg.value().substr(0u, pos), arg.value().substr(pos + 1));
				else
					throw std::exception(std::string("Invalid \"--set\" command: \"" + arg.value() + "\"").c_str());
			}
		}*/
		// if the INI configuration has changed, write it to file
		//if (update_ini_before_return) {
		if (modified) {
			if (file::write_to(filename, gs.to_stream(), false))
				std::cout << term::msg << "Successfully wrote to \"" << filename << '\"' << std::endl;
			else
				std::cout << term::warn << "Failed to write to \"" << filename << '\"' << std::endl;
		}
		if (const auto get_args{ args.get_all<opt3::Option>("get")}; !get_args.empty()) {
			const bool print_all{ std::any_of(get_args.begin(), get_args.end(), [](auto&& opt) { return !opt.has_value(); }) };
			if (print_all) for (auto& it : gs)
				std::cout << it._name << " = " << it.safe_get() << '\n';
			else for (auto& arg : get_args) {
				const auto capv{ arg.value() };
				if (const auto target{ gs.find(capv, 0, true) }; target != gs.end())
					std::cout << target->_name << " = " << target->safe_get() << '\n';
				else
					std::cout << term::warn << "\"" << capv << "\" not found.\n";
			}
		}
		return gs; // RETURN
	}
	/**
	 * @struct Instance
	 * @brief Represents an instance of the caco_alch program. Contains all the high-level methods used to run the program.
	 */
	struct Instance {
		using ConfigType = std::optional<file::ini::INI>;

		std::string argv0;
		opt3::ArgManager Arguments;
		ConfigPathList Paths;
		ConfigType Config;
		Alchemy alchemy;

		Instance(std::string argv0, opt3::ArgManager args, ConfigPathList paths) :
			argv0{ argv0 },
			Arguments{ std::move(args) },
			Paths{ std::move(paths) },
			Config{ [this]() -> ConfigType {
			const auto iniPath{ Arguments.getv<opt3::Option>("config").value_or(Paths.ini.generic_string()) };
			return file::exists(iniPath) ? file::ini::INI(iniPath) : static_cast<ConfigType>(std::nullopt);
		}() },
			alchemy{ loadFromFile(Paths.ingredients), { Arguments, Config }, loadGameConfig(Paths.gameconfig, Arguments, DefaultObjects._settings) } {}


		const static int RETURN_SUCCESS{ 0 };
		const static int RETURN_FAILURE{ 1 };

		/**
		 * @brief Validate the location of the configuration files used by the program.
		 * @param os	 - Target output stream.
		 * @param indent - Maximum indentation value, from which the length of all prefixes are subtracted to create uniform aligned values.
		 */
		void validate(std::ostream& os, const env::PATH& path, const std::streamsize indent = 20ll) const
		{
			const auto print{ [&os, &indent](const std::string& name, const std::string& target) {
				os << name << format::indent(indent, name.size()) << (file::exists(target) ? color::setcolor::green : color::setcolor::red) << target << color::reset << '\n';
			} };
			print("argv[0]", argv0);
			print("directory", Paths.localDir.generic_string());
			print("registry", Paths.ingredients.generic_string());
			print("INI Config", Paths.ini.generic_string());
			print("Game Config", Paths.gameconfig.generic_string());
		}
		/**
		 * @brief Validate the location of the configuration files used by the program and print it to std::cout.
		 */
		void validate(const env::PATH& path) const { validate(std::cout, path); }
		/**
		 * @brief Handle the received commandline arguments and call the relevant functions with the remaining parameters.
		 * @param os	- Target output stream.
		 * @returns int
		 */
		int handleArguments(std::ostream& os) const
		{
			// i - Build from File
			if (Arguments.check<opt3::Flag>('i')) {
				std::stringstream buffer;
				buffer << std::cin.rdbuf();
				alchemy.print_build(os, parseFileContent(buffer)).flush();
				return RETURN_SUCCESS;
			}
			// l - List
			else if (Arguments.check<opt3::Flag>('l')) {
				alchemy.print_list(os).flush();
				return RETURN_SUCCESS;
			}
			else {
				// b - Build
				const auto params{ Arguments.getv_all<opt3::Parameter>() };
				if (Arguments.check<opt3::Flag>('b')) {
					alchemy.print_build(os, params, 4u).flush();
					return RETURN_SUCCESS;
				}
				// S - Smart Search
				else if (Arguments.check<opt3::Flag>('S')) {
					alchemy.print_smart_search(os, params).flush();
					return RETURN_SUCCESS;
				}
				// s - Search
				else if (Arguments.check<opt3::Flag>('s')) {
					if (const auto mag{ Arguments.check<opt3::Flag>('m') }, dur{ Arguments.check<opt3::Flag>('d') }, ranked{ Arguments.check<opt3::Flag>('r') }; mag || dur || ranked) {
						RegistryType::FXFindType ft;
						if (mag && dur)
							ft = RegistryType::FXFindType::BOTH_AND;
						else if (mag)
							ft = RegistryType::FXFindType::MAG;
						else if (dur)
							ft = RegistryType::FXFindType::DUR;
						else
							ft = RegistryType::FXFindType::BOTH_OR;
						for (auto& arg : params) {
							if (ranked)
								alchemy.print_ranked_best(os, arg, ft);
							else
								alchemy.print_best(os, arg, ft);
						}
					}
					else for (auto& arg : params)
						alchemy.print_search(os, arg).flush();
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
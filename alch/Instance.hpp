#pragma once
#include <fileio.hpp>
#include <fileutil.hpp>
//#include <resolve-path.hpp>
#include <color-transform.hpp>
#include <INI.hpp>

#include <env.hpp>
#include <str.hpp>

#include <Alchemy.hpp>
#include <GameConfig.hpp>
#include <reloader.hpp>
#include <handle_arguments.hpp>
#include <DefaultObjects.hpp>
//#include <ColorConfigLoader.hpp>


namespace caco_alch {
	template<class T>
	struct ModTrigger : public T {
		bool _armed, _triggered;
	public:
		using ParentType = T;
		template<class... VT>
		ModTrigger(VT&&... args) : ParentType(std::forward<VT>(args)...) {}

		bool isArmed() const { return _armed; }
		bool isTriggered() const { return _triggered; }

		void arm() { _armed = true; }
		void disarm() { _armed = false; }

		void trigger() { if (_armed) _triggered = true; }

		operator T& () { trigger(); return *this; }
		auto& get() { return operator T & (); }
	};

	struct GS : public ModTrigger<GameConfig> {

	};
	/**
	 * @brief
	 * @param filename
	 * @param args
	 * @param defaults	Default GameConfig Values
	 * @returns			GameConfig
	 */
	inline GameConfig loadGameConfig(const std::filesystem::path& filename, const opt::ParamsAPI2& args, const GameConfig::Cont& defaults)
	{
		//GameConfig gs(defaults); // INIT
		GS gs{ defaults };

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
			else throw make_exception("Missing argument for \"--set\" option.");
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
		ConfigPathList Paths;
		ConfigType Config;
		Alchemy Alchemy;

		Instance(opt::ParamsAPI2 args, ConfigPathList paths) :
			Arguments{ std::move(args) },
			Paths{ std::move(paths) },
			Config{ [this]() -> ConfigType {
			const auto iniPath{ (Arguments.check<opt::Option>(DefaultObjects._load_config) ? Arguments.typegetv<opt::Option>(DefaultObjects._load_config).value_or(Paths.ini.generic_string()) : Paths.ini) };
			return file::exists(iniPath) ? file::ini::INI(iniPath) : static_cast<ConfigType>(std::nullopt);
		}() },
			Alchemy{ loadFromFile(Paths.ingredients), { Arguments, Config }, loadGameConfig(Paths.gameconfig, Arguments, DefaultObjects._settings) } {}


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
				os << name << str::VIndent(indent, name.size()) << (file::exists(target) ? color::f::green : color::f::red) << target << color::reset << '\n';
			} };
			print("argv[0]", Arguments.arg0().value_or(""));
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
			if (Arguments.check<opt::Flag>('i')) {
				std::stringstream buffer;
				buffer << std::cin.rdbuf();
				Alchemy.print_build(os, parseFileContent(buffer)).flush();
				return RETURN_SUCCESS;
			}
			// l - List
			else if (Arguments.check<opt::Flag>('l')) {
				Alchemy.print_list(os).flush();
				return RETURN_SUCCESS;
			}
			else {
				// b - Build
				const auto params{ Arguments.typegetv_all<opt::Parameter>() };
				if (Arguments.check<opt::Flag>('b')) {
					Alchemy.print_build(os, params, 4u).flush();
					return RETURN_SUCCESS;
				}
				// S - Smart Search
				else if (Arguments.check<opt::Flag>('S')) {
					Alchemy.print_smart_search(os, params).flush();
					return RETURN_SUCCESS;
				}
				// s - Search
				else if (Arguments.check<opt::Flag>('s')) {
					for (auto& arg : params)
						Alchemy.print_search(os, arg).flush();
					return RETURN_SUCCESS;
				}
				// m - Highest Magnitude Search
				else if (Arguments.check<opt::Flag>('m')) {
					for (auto& arg : params)
						Alchemy.print_best(os, arg, { "jarrin root", "chokeberries" }).flush();
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
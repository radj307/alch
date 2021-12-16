#pragma once
#include <ParamsAPI2.hpp>
#include <GameConfig.hpp>
#include <ColorAPI.hpp>
#include <fileutil.hpp>

#include <string>
#include <filesystem>
#include <vector>

namespace caco_alch {
	struct ConfigPathList {
		using PathT = std::filesystem::path;
		PathT localDir, ini, gameconfig, ingredients;

		ConfigPathList(const PathT& local_dir, const PathT& ini, const PathT& gameconfig, const PathT& ingredients)
			: localDir{ local_dir }, ini{ ini }, gameconfig{ gameconfig }, ingredients{ ingredients } {}

		/**
		 * @brief Checks if the given paths exist, and returns the highest priority extant path, or nullopt.
		 * @param path		- Path defined by the user on the commandline. Accepts an optional string.
		 * @param defPath	- Default path located in DefaultPaths instance.
		 * @returns std::optional<std::string>
		 */
		static std::optional<std::string> resolve_path(const std::optional<std::string>& path, const std::string& defPath)
		{
			if (path.has_value() && file::exists(path.value())) // first check the user-defined path
				return path;
			else if (file::exists(defPath)) // second check the default path
				return defPath;
			else // if neither exist, return nullopt
				return std::nullopt;
		}
	};

	/**
	 * @struct DefaultObjects
	 * @brief Contains the overridable default objects and values used in various parts of the program.
	 */
	static struct {
		const std::string
			_default_filename_config{ "alch.ini" },
			_default_filename_gamesettings{ "alch.gamesetting.ini" },
			_default_filename_registry{ "alch.ingredients" };

		const std::string
			_help{ "help" },
			_load_config{ "load-ini" },
			_load_gamesettings{ "load-gameconfig" },
			_load_registry{ "load-registry" };
		const std::string
			_reset_gamesettings{ "reset-gameconfig" };
		const std::string
			_set_gamesetting{ "set" },
			_get_gamesetting{ "get" };

		const std::vector<std::pair<std::string, std::vector<std::pair<std::string, std::string>>>> _help_doc{
			{ "MODES"s, {
				{ "-l", "List all ingredients in the registry." },
				{ "-s", "Search mode. Accepts any number of ingredient and/or effect names." },
				{ "-S", "Smart Search mode. Accepts 2 to 4 effect names, and shows ingredients that have all of the searched effects." },
				{ "-b", "Build mode. Accepts 2 to 4 ingredient names, and shows the potion that would result from combining them." },
				{ "-i", "Build-from-file mode. Receives a list of ingredients from STDIN (cat command), and automatically attempts to build them. See the -E modifier for more." },
			} },
			{ "MODIFIERS"s, {
				{ "-e", "Only allow exact matches." },
				{ "-q", "Quiets extra output, in most cases this hides all non-searched effects for printed ingredients." },
				{ "-v", "Includes keywords when printing effects." },
				{ "-a", "Show all information in output." },
				{ "-E", "Changes all output to use file formatting to allow piping output to and from files with '-i'" },
				{ "-R", "Reverse ordering. This is reverse-alphabetical unless using the '-r' option." },
				{ "-r", "(search) Ranked ordering. Displays ingredients with matching effects in order of magnitude/duration." },
				{ "",	"By default, ingredients must have higher magnitude OR duration to be considered better. Set -m and/or -d to change this behavior." },
				{ "-m", "(search) Orders by magnitude. When specified with duration, ingredients must have higher magnitude AND duration to be considered better." },
				{ "-d", "(search) Orders by duration. When specified with magnitude, ingredients must have higher magnitude AND duration to be considered better." },
			} },
			{ "OPTIONS"s, {
				{ "--" + _load_registry + " <file>", "Loads the specified registry file instead of the default one." },
				{ "--" + _load_config + " <file>", "Loads the specified configuration file instead of the default one." },
				{ "--" + _load_gamesettings + " <file>", "Loads the specified game config file instead of the default one." },
				{ "--" + _reset_gamesettings, "Reset or create the default game config file." },
				{ "--" + _set_gamesetting + " <setting>:<value>", "Set the value of the specified game config setting." },
				{ "--" + _get_gamesetting + " [setting]", "Retrieve the value of a game config setting. If no setting is specified, all settings are shown." },
				{ "-h  --" + _help, "Shows this help display." },
				{ "-c", "Enables colorization of effect names based on whether they have positive/negative/neutral keywords." },
				{ "--precision <uint>", "Set the number of digits after the decimal point that should be shown. Default is 2." },
				{ "--validate", "Shows debug information including filepaths, and whether they were found (green) or not (red)." },
			} },
		};

		const GameConfig::Cont _settings{
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

	class Help {
		const std::string _programName;
	public:
		constexpr Help(const std::string& program_name = "alch") : _programName{ program_name } {}

		friend std::ostream& operator<<(std::ostream& os, const Help& doc)
		{
			os << "USAGE:\n  " << doc._programName << " <MODE> [OPTIONS|MODIFIERS] [NAMES]\n\n";
			for (auto& [name, section] : DefaultObjects._help_doc) {
				os << name << "\n";
				const auto longest{ str::longest<0ull>(section)->first.size() + 2ull };
				for (auto& [opt, desc] : section) {
					os << "  " << opt << str::VIndent(longest, opt.size()) << desc << '\n';
				}
				os << '\n';
			}
			return os;
		}
	};
}
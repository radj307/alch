#pragma once
#include <string>
#include <vector>
#include <Params.hpp>
#include <Help.hpp>
#include <GameConfig.hpp>
#include <ColorAPI.hpp>

namespace caco_alch {

	struct DefaultPaths {
	private:
		static std::string combine(std::string left, std::string&& right)
		{
			return left.append(std::move(right));
		}
	public:
		DefaultPaths(std::string local_path, std::string config, std::string gamesettings, std::string registry) : _path_config{ combine(local_path, std::move(config)) }, _path_gamesettings{ combine(local_path, std::move(gamesettings)) }, _path_registry{ combine(local_path, std::move(registry)) } {}
		std::string
			_path_config,
			_path_gamesettings,
			_path_registry;

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
	struct {
		const std::string
			_default_filename_config{ "alch-Config.ini" },
			_default_filename_gamesettings{ "alch-GameConfigBase.ini" },
			_default_filename_registry{ "alch.ingredients" };

		const std::string
			_help{ "help" },
			_load_config{ "load-ini" },
			_load_gamesettings{ "load-gameconfig" },
			_load_registry{ "load-registry" };
		const std::string
			_reset_gamesettings{ "reset-gameconfig" };
		const std::string
			_set_gamesetting{ "set" };

		const std::vector<std::string> _matcher{
			"color",
			"precision",
			_set_gamesetting,
			_load_config,
			_load_gamesettings,
			_load_registry
		};

		opt::help::Doc _help_doc{ "alch", "[OPTIONS]... [INGR|EFFECT]...", "Commandline potion builder utility for Skyrim.", {
				{ "Mututally Exclusive Mode Options", {
					{ "-l", "List all ingredients in the registry." },
					{ "-s", "Search mode. Accepts any number of ingredient and/or effect names." },
					{ "-S", "Smart Search mode. Accepts 2 to 4 effect names, and shows ingredients that have all of the searched effects." },
					{ "-b", "Build mode. Accepts 2 to 4 ingredient names, and shows the potion that would result from combining them." },
					{ "-i", "Build-from-file mode. Receives a list of ingredients from STDIN (cat command), and automatically attempts to build them. See the -E option for more." },
				} },
				{ "Modifier Options", {
					{ "-a", "All modifier. Shows all additional information." },
					{ "-v", "Verbose modifier. Shows effect keywords." },
					{ "-q", "Quiet modifier. Only shows results that were explicitly specified." },
					{ "-e", "Exact modifier. Only allows whole-word matches. (Searches are always case-insensitive)" },
					{ "-E", "Export modifier. Prints output in registry-file-format for use with pipe operators and the -i option." },
					{ "-R", "Reverse modifier. Prints output in reverse-alphabetical order." },
				} },
				{ "Configuration / File Overrides", {
					{ "--" + _load_registry + " <file>", "Loads the specified registry file instead of the default one." },
					{ "--" + _load_config + " <file>", "Loads the specified configuration file instead of the default one."},
					{ "--" + _load_gamesettings + " <file>", "Loads the specified game config file instead of the default one."},
					{ "--" + _reset_gamesettings, "Reset or create the default game config file." },
					{ "--" + _set_gamesetting + " <setting>:<value>", "Allows modifying the game config directly from the commandline." },
				} },
				{ "Help / Debug", {
					{ "-h" },{ "--" + _help, "Shows this help display." },
					{ "--validate", "Shows debug information including filepaths, and whether they were found (green) or not (red)." },
				} },
				{ "Appearance", {
					{ "-c", "Enables colorization of effect names based on whether they have positive/negative/neutral keywords." },
					{ "--precision <uint>", "Set the number of digits after the decimal point that should be shown. Default is 2." },
					{ "--color <string>", "Allows changing the color of ingredient names." },
				} }
			}
		};
		const ColorAPI::PaletteType _default_colors{
			{ UIElement::BRACKET,				Color{ color::red, true } },
			{ UIElement::SEARCH_HIGHLIGHT,		Color{ color::yellow} },
			{ UIElement::SEARCH_HEADER,			Color{ color::intense_white } },
			{ UIElement::INGREDIENT_NAME,		Color{ color::white, true } },
			{ UIElement::POTION_NAME,			Color{ color::white, true } },
			{ UIElement::ALCHEMY_SKILL,			Color{ color::cyan } },
			{ UIElement::EFFECT_NAME_DEFAULT,	Color{ color::white } },
			{ UIElement::EFFECT_NAME_POSITIVE,	Color{ color::green } },
			{ UIElement::EFFECT_NAME_NEGATIVE,	Color{ color::red } },
			{ UIElement::EFFECT_NAME_NEUTRAL,	Color{ color::white } },
			{ UIElement::EFFECT_MAGNITUDE,		Color{ color::magenta } },
			{ UIElement::EFFECT_DURATION,		Color{ color::cyan } },
			{ UIElement::KEYWORD,				Color{ color::gray } },
		};
		ColorAPI::PaletteType::const_iterator getDefaultUIColor(const UIElement& ui) const
		{
			for (auto it{ _default_colors.begin() }; it != _default_colors.end(); ++it)
				if (ui == it->first)
					return it;
			return _default_colors.end();
		}
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
}
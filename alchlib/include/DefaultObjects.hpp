#pragma once
#include "GameConfig.hpp"
#include "ColorAPI.hpp"

#include <TermAPI.hpp>
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

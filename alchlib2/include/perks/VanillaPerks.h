#pragma once
#include "../Effect.hpp"
#include "../Potion.hpp"
#include "../PerkBase.hpp"
#include "../keywords/VanillaKeywords.h"

#include <nlohmann/json.hpp>

namespace alchlib2::perks {
	struct AlchemistPerk : PerkBase {
		static constexpr const auto Name{ "Alchemist" };

		/**
		 * @brief			Potions and poisons you make are stronger depending on rank.
		 * @param rank		The vanilla game allows ranks 1 (20%) through 5 (100%).
		 */
		AlchemistPerk(const std::uint8_t& rank = 1) : PerkBase(Name), rank{ rank } {}

		/// @brief	The vanilla game allows ranks 1 through 5.
		std::uint8_t rank;

		void ApplyToPotion(Potion& potion) const noexcept override
		{
			potion.ModAllMagnitudes(0.2f * $c(float, rank));
		}

		NLOHMANN_DEFINE_TYPE_INTRUSIVE(AlchemistPerk, name, rank);
	};
	struct PhysicianPerk : PerkBase {
		static constexpr const auto Name{ "Physician" };

		PhysicianPerk() : PerkBase(Name) {}

		void ApplyToPotion(Potion& potion) const noexcept override
		{
			if (potion.AnyEffectHasKeyword(alchlib2::keywords::MagicAlchRestoreHealth, alchlib2::keywords::MagicAlchRestoreStamina, alchlib2::keywords::MagicAlchRestoreMagicka)) {
				potion.ModAllMagnitudes(1.25f);
			}
		}

		NLOHMANN_DEFINE_TYPE_INTRUSIVE(PhysicianPerk, name);
	};
	struct BenefactorPerk : PerkBase {
		static constexpr const auto Name{ "Benefactor" };

		/// @brief	Potions you mix with beneficial effects have an additional 25% greater magnitude.
		BenefactorPerk() : PerkBase(Name) {}

		void ApplyToPotion(Potion& potion) const noexcept override
		{
			if (potion.AnyEffectHasKeyword(alchlib2::keywords::MagicAlchBeneficial) && !potion.IsPoison()) {
				potion.ModAllMagnitudes(1.25f);
			}
		}

		NLOHMANN_DEFINE_TYPE_INTRUSIVE(BenefactorPerk, name);
	};
	struct PoisonerPerk : PerkBase {
		static constexpr const auto Name{ "Poisoner" };

		PoisonerPerk() : PerkBase(Name) {}

		void ApplyToPotion(Potion& potion) const noexcept override
		{
			if (potion.AnyEffectHasKeyword(alchlib2::keywords::MagicAlchHarmful) && potion.IsPoison()) {
				potion.ModAllMagnitudes(1.25f);
			}
		}

		NLOHMANN_DEFINE_TYPE_INTRUSIVE(PoisonerPerk, name);
	};
	struct PurityPerk : PerkBase {
		static constexpr const auto Name{ "Purity" };

		PurityPerk() : PerkBase(Name) {}

		void ApplyToPotion(Potion& potion) const noexcept override
		{
			if (potion.IsPoison())
				potion.effects.erase(std::remove_if(potion.effects.begin(), potion.effects.end(), [](auto&& effect) { return effect.HasAnyKeyword(alchlib2::keywords::MagicAlchBeneficial); }), potion.effects.end());
			else
				potion.effects.erase(std::remove_if(potion.effects.begin(), potion.effects.end(), [](auto&& effect) { return effect.HasAnyKeyword(alchlib2::keywords::MagicAlchHarmful); }), potion.effects.end());
		}

		NLOHMANN_DEFINE_TYPE_INTRUSIVE(PurityPerk, name);
	};

	struct VanillaPerks {
		AlchemistPerk Alchemist;
		PhysicianPerk Physician;
		BenefactorPerk Benefactor;
		PoisonerPerk Poisoner;
		PurityPerk Purity;

		NLOHMANN_DEFINE_TYPE_INTRUSIVE(VanillaPerks, Alchemist, Physician, Benefactor, Poisoner, Purity);

		std::vector<Perk> GetAllPerks() const
		{
			std::vector<Perk> perks;
			perks.reserve(5);

			if (Alchemist.enable) perks.emplace_back(Alchemist);
			if (Physician.enable) perks.emplace_back(Physician);
			if (Benefactor.enable) perks.emplace_back(Benefactor);
			if (Poisoner.enable) perks.emplace_back(Poisoner);
			if (Purity.enable) perks.emplace_back(Purity);

			perks.shrink_to_fit();
			return perks;
		}

		static VanillaPerks ReadFrom(std::filesystem::path const& path)
		{
			nlohmann::json j;
			file::read(path) >> j;
			return j.get<VanillaPerks>();
		}
		static bool WriteTo(std::filesystem::path const& path, VanillaPerks const& perks)
		{
			return file::write(path, nlohmann::json{ perks });
		}
	};
}
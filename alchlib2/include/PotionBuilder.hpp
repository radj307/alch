#pragma once
#include "Potion.hpp"
#include "Formula.hpp"
#include "PerkBase.hpp"

#include "perks/VanillaPerks.h"

namespace alchlib2 {
	/**
	 * @brief		Retrieve a list of common effects with the strongest available magnitude & duration from the given Ingredient list.
	 * @param ingr	List of ingredients
	 * @returns		std::vector<Effect>
	 */
	static CONSTEXPR std::vector<Effect> get_common_effects(const std::vector<Ingredient>& ingr)
	{
		std::vector<Effect> common, tmp;
		constexpr auto is_duplicate{ [](std::vector<Effect>& target, const Effect& fx) {
			for (auto it{ target.begin() }; it != target.end(); ++it)
				if (it->name == fx.name) // if effect names are the same, consider it a duplicate even though the magnitudes might be different
					return it;
			return target.end();
		} };

		for (auto& i : ingr) {
			for (auto& it : i.effects) {
				if (auto dupl{ is_duplicate(tmp, it) }; dupl != tmp.end()) { // if effect is a duplicate, push it to the common effects vector
					if (auto current{ is_duplicate(common, it) }; current == common.end()) {
						if (it.magnitude < dupl->magnitude)
							common.push_back(*dupl); // if effect is not in the common list yet, add it
						else
							common.push_back(it);
					}
					else {
						if (it.magnitude > current->magnitude)	// Set magnitude to largest (base_mag)
							(*current).magnitude = it.magnitude;
						if (it.duration > current->duration)	// Set duration to largest (base_dur)
							(*current).duration = it.duration;
					}
				}
				else
					tmp.emplace_back(it);
			}
		}
		return common;
	}

	struct PotionBuilder {
		AlchemyCoreFormula coreFormula;

		PotionBuilder(AlchemyCoreFormula const& coreFormula) : coreFormula{ coreFormula } {}
		PotionBuilder(AlchemyCoreGameSettings const& coreGameSettings) : coreFormula{ coreGameSettings } {}

		[[nodiscard]] std::string GetNameFromEffects(std::vector<Effect> const& effects) const
		{
			std::string name;
			auto strongest{ effects.end() };
			for (auto it{ effects.begin() }; it != effects.end(); ++it)
				if (strongest == effects.end() || it->magnitude > strongest->magnitude)
					strongest = it;
			if (strongest != effects.end()) {
				name = " of " + strongest->name;
				if (const auto& disposition{ strongest->GetDisposition() }; disposition >= EKeywordDisposition::Negative)
					name = "Poison" + name;
				else if (effects.size() > 2)
					name = "Elixir" + name;
				else if (effects.size() == 2)
					name = "Draught" + name;
				else
					name = "Potion" + name;
			}
			else
				name = "Potion";
			return name;
		}
		[[nodiscard]] Potion Build(std::vector<Ingredient> const& ingredients, std::vector<Perk> const& perks) const
		{
			auto common{ get_common_effects(ingredients) };
			for (auto& effect : common) {
				if (effect.HasAnyKeyword(keywords::MagicAlchDurationBased))
					effect.duration = std::round(coreFormula.GetResult(effect.magnitude));
				else
					effect.magnitude = std::round(coreFormula.GetResult(effect.magnitude));

				for (const auto& it : perks)
					it.ApplyToEffect(effect);
			}
			Potion p{ GetNameFromEffects(common), common };

			for (const auto& it : perks)
				it.ApplyToPotion(p);

			return p;
		}
		template<var::any_same_or_convertible<Perk>... TPerks>
		[[nodiscard]] Potion Build(std::vector<Ingredient> const& ingredients, TPerks&&... perks) const
		{
			return Build(ingredients, { std::forward<TPerks>(perks)... });
		}
	};
}

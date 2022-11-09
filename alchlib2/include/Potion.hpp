#pragma once
#include "Effect.hpp"
#include "Ingredient.hpp"
#include "Formula.hpp"

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

	struct Potion : INamedObject {
		std::vector<Ingredient> ingredients;

		STRCONSTEXPR Potion() {}
		STRCONSTEXPR Potion(std::string const& name, std::vector<Ingredient> ingredients = {}) : INamedObject(name), ingredients{ ingredients } {}

		[[nodiscard]] CONSTEXPR std::vector<Effect> GetCommonEffects() const noexcept
		{
			return get_common_effects(ingredients);
		}
		template<typename... Ts>
		[[nodiscard]] CONSTEXPR std::vector<Effect> GetCalculatedEffects(FormulaBase<float, float, Ts...>* formula) const noexcept
		{
			if (formula == nullptr) return{};
			auto common{ get_common_effects(ingredients) };
			// TODO: implement this
			return common;
		}
	};
}

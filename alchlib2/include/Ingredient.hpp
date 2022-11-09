#pragma once
#include "INamedObject.hpp"
#include "Effect.hpp"

#include <algorithm>

namespace alchlib2 {
	struct Ingredient : INamedObject {
		std::vector<Effect> effects;

		STRCONSTEXPR Ingredient() {}
		STRCONSTEXPR Ingredient(std::string const& name, const std::vector<Effect>& effects = {}) : INamedObject(name), effects{effects} {}

		[[nodiscard]] CONSTEXPR bool IsSimilarTo(const std::string& name) const
		{
			const auto thisNameLower{ str::tolower(this->name) }, nameLower{ str::tolower(name) };
			return thisNameLower.find(nameLower) != std::string::npos;
		}

		[[nodiscard]] CONSTEXPR bool AnyEffectIsSimilarTo(const std::string& name) const
		{
			return std::any_of(effects.begin(), effects.end(), [&](auto&& effect) {
				return effect.IsSimilarTo(name);
			});
		}

		[[nodiscard]] CONSTEXPR bool AnyEffectKeywordIsSimilarTo(const std::string& name) const
		{
			return std::any_of(effects.begin(), effects.end(), [&name](auto&& effect) {
				return std::any_of(effect.keywords.begin(), effect.keywords.end(), [&name](auto&& keyword) {
					return keyword.IsSimilarTo(name);
				});
			});
		}

		[[nodiscard]] Ingredient MaskEffects(const std::function<bool(Effect)>& pred)
		{
			Ingredient copy{ *this };
			copy.effects.erase(std::remove_if(copy.effects.begin(), copy.effects.end(), pred), copy.effects.end());
			return copy;
		}
	};
}

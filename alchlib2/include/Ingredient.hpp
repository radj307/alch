#pragma once
#include "INamedObject.hpp"
#include "Effect.hpp"

#include <algorithm>

namespace alchlib2 {
	struct Ingredient : INamedObject {
		std::vector<Effect> effects;

		STRCONSTEXPR Ingredient() = default;
		STRCONSTEXPR Ingredient(std::string const& name, const std::vector<Effect>& effects = {}) : INamedObject(name), effects{ effects } {}

	#pragma region IsSimilarTo
		[[nodiscard]] CONSTEXPR bool IsSimilarTo(std::string name, const bool requireExactMatch) const
		{
			name = str::tolower(name);
			const auto thisNameLower{ str::tolower(this->name) };
			return requireExactMatch ? name == thisNameLower : thisNameLower.find(name) != std::string::npos;
		}

		[[nodiscard]] CONSTEXPR bool AnyEffectIsSimilarTo(std::string name, const bool requireExactMatch) const
		{
			name = str::tolower(name);
			return std::any_of(effects.begin(), effects.end(), [&name, &requireExactMatch](auto&& effect) {
				return requireExactMatch ? str::tolower(effect.name) == name : effect.IsSimilarTo(name, requireExactMatch);
			});
		}

		[[nodiscard]] CONSTEXPR bool AnyEffectKeywordIsSimilarTo(std::string name, const bool requireExactMatch) const
		{
			name = str::tolower(name);
			return std::any_of(effects.begin(), effects.end(), [&name, &requireExactMatch](auto&& effect) -> bool {
				return std::any_of(effect.keywords.begin(), effect.keywords.end(), [&name, &requireExactMatch](auto&& keyword) -> bool {
					return requireExactMatch ? str::tolower(keyword.name) == name : keyword.IsSimilarTo(name, requireExactMatch);
				});
			});
		}
	#pragma endregion IsSimilarTo

		CONSTEXPR auto operator<=>(const Ingredient& o) const noexcept
		{
			return str::tolower(name).compare(str::tolower(o.name));
		}

		[[nodiscard]] Ingredient MaskEffects(const std::function<bool(Effect)>& pred)
		{
			Ingredient copy{ *this };
			copy.effects.erase(std::remove_if(copy.effects.begin(), copy.effects.end(), pred), copy.effects.end());
			return copy;
		}
	};
}

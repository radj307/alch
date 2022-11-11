#pragma once
#include "Effect.hpp"
#include "keywords/VanillaKeywords.h"

namespace alchlib2 {
	struct Potion : INamedObject {
		std::vector<Effect> effects;

		STRCONSTEXPR Potion() {}
		STRCONSTEXPR Potion(std::string const& name, std::vector<Effect> const& effects) : INamedObject(name), effects{ effects } {}

		[[nodiscard]] Effect GetStrongestEffect() const noexcept
		{
			std::vector<Effect>::const_iterator strongest;
			for (auto it{ effects.begin() }; it != effects.end(); ++it) {
				if (strongest == effects.end() || it->magnitude > strongest->magnitude) {
					strongest = it;
				}
			}
			return *strongest;
		}

		[[nodiscard]] bool IsPoison() const noexcept
		{
			return GetStrongestEffect().HasAnyKeyword(keywords::MagicAlchHarmful);
		}

		template<var::any_same_or_convertible<Keyword>... TKeywords> requires var::at_least_one<TKeywords...>
		[[nodiscard]] CONSTEXPR bool AnyEffectHasKeyword(TKeywords const&... keywords)
		{
			return std::any_of(effects.begin(), effects.end(), [&](Effect const& effect) -> bool {
				return effect.HasAnyKeyword(keywords...);
			});
		}

		[[nodiscard]] CONSTEXPR void ModAllMagnitudes(const float multiplier)
		{
			for (auto& effect : effects) {
				effect.magnitude *= multiplier;
			}
		}
		[[nodiscard]] CONSTEXPR void ModAllDurations(const float multiplier)
		{
			for (auto& effect : effects) {
				effect.duration = $c(unsigned, std::round($c(float, effect.duration) * multiplier));
			}
		}
	};
}

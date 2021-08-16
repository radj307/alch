#pragma once
#include <algorithm>
#include <utility>

#include "using.h"

namespace caco_alch {
	struct Perk : ObjectBase {
		KeywordList _valid_keywords;
		Perk(const std::string& name, KeywordList valid_keywords) : ObjectBase(name), _valid_keywords{ std::move(valid_keywords) } {}

		[[nodiscard]] bool is_valid_target(const Effect& effect) const
		{
			return std::any_of(_valid_keywords.begin(), _valid_keywords.end(), [&effect](const Keyword& KYWD){ return effect.hasKeyword(KYWD); });
		}
	};

	const std::vector<Perk> _perks{
		{ "Poisoner", KeywordList{ Keyword{ "MagicAlchHarmful" } } },
		{ "Benefactor", KeywordList{ Keyword{ "MagicAlchBeneficial" } } },
	};
}
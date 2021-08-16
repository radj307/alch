#pragma once
#include <optional>
#include "using.h"

namespace caco_alch {
	struct Perk : ObjectBase {
		KeywordList _valid_keywords;
		Perk(const std::string& name, const KeywordList& valid_keywords) : ObjectBase(name), _valid_keywords{ valid_keywords } {}

		bool is_valid_target(const Effect& effect) const
		{
			for ( auto& KYWD : _valid_keywords )
				if ( effect.hasKeyword(KYWD) )
					return true;
			return false;
		}
	};

	const std::vector<Perk> _perks{
		{ "Poisoner", { { "MagicAlchHarmful" } } },
		{ "Benefactor", { { "MagicAlchBeneficial" } } },
	};
}
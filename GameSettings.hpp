#pragma once
#include <cmath>

namespace caco_alch {
	struct GameSettings {
		double
			_fAlchemyIngredientInitMult{ 3.0 },	///< @brief Game Setting that controls the overall strength of player-made potions.
			_fAlchemySkillFactor{ 3.0 };		///< @brief Game Setting that controls how much stats are improved based on alchemy skill level.
		size_t _alchemy_skill{ 15u };			///< @brief The player's current, or assumed, skill level in Alchemy.
		size_t _duration{ 1u };					///< @brief Default potion duration.

		double potion_calc_magnitude(const double base_mag) const { return std::round(base_mag * 4.0 * _fAlchemyIngredientInitMult * ( 1.0 + ( _fAlchemySkillFactor - 1.0 ) * ( _alchemy_skill / 100 ) )); }

		GameSettings() = default;
		GameSettings(const double fAlchemyIngredientInitMult, const double fAlchemySkillFactor, const size_t alchemy_skill_level, const size_t duration) : _fAlchemyIngredientInitMult{ fAlchemyIngredientInitMult }, _fAlchemySkillFactor{ fAlchemySkillFactor }, _alchemy_skill{ alchemy_skill_level }, _duration{ duration } {}
	};
}
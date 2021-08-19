#pragma once
#include <utility>

#include "using.h"
#include "Ingredient.hpp"
#include "GameSettings.hpp"

namespace caco_alch {

	/**
	 * @function get_common_effects(const std::vector<Ingredient>&)
	 * @brief Retrieve a list of common effects with the magnitude of the strongest effect of that type. (Base Magnitude)
	 * @param ingr	- List of ingredients
	 * @returns EffectList
	 */
	static EffectList get_common_effects(const IngrList&ingr)
	{
		if ( ingr.size() > 4 ) throw std::exception("Too many ingredients! (Max 4)"); else if ( ingr.size() < 2 ) throw std::exception("Not enough ingredients! (Min 2)");
		EffectList common, tmp;
		constexpr auto is_duplicate{ [](EffectList& target, const Effect& fx) {
			for ( auto it{ target.begin() }; it != target.end(); ++it )
				if ( it->_name == fx._name ) // if effect names are the same, consider it a duplicate even though the magnitudes might be different
					return it;
			return target.end();
		} };

		for ( auto& i : ingr )
			for ( auto& it : i._effects ) {
				if ( auto dupl{ is_duplicate(tmp, it) }; dupl != tmp.end() ) { // if effect is a duplicate, push it to the common effects vector
					if ( auto current{ is_duplicate(common, it) }; current == common.end() ) {
						if ( it._magnitude < dupl->_magnitude )
							common.push_back(*dupl); // if effect is not in the common list yet, add it
						else
							common.push_back(it);
					}
					else {
						if ( it._magnitude > current->_magnitude)	// Set magnitude to largest (base_mag)
							( *current )._magnitude = it._magnitude;
						if ( it._duration > current->_duration )	// Set duration to largest (base_dur)
							( *current )._duration = it._duration;
					}
				}
				else
					tmp.emplace_back(it);
			}
		return common;
	}

	/**
	 * @struct PotionBase
	 * @brief Contains the base effects of a potion, before using GameSettings to calculate the expected resulting magnitude for each effect.
	 */
	struct PotionBase : ObjectBase {
	protected:
		EffectList _base_fx; ///< @brief The base effects of a potion, before calculating the potion magnitude.

		static std::string getName(const EffectList& effects)
		{
			auto strongest{ effects.end() };
			for ( auto it{ effects.begin() }; it != effects.end(); ++it )
				if ( strongest == effects.end() || it->_magnitude > strongest->_magnitude )
					strongest = it;
			if ( strongest != effects.end() ) {
				std::string name{ " of " + strongest->_name };
				if ( hasNegative(*strongest) )
					name = "Poison" + name;
				else if ( effects.size() > 2 )
					name = "Elixir" + name;
				else if ( effects.size() == 2 )
					name = "Draught" + name;
				else
					name = "Potion" + name;
				return name;
			}
			return "Potion";
		}

	public:
		explicit PotionBase(const std::string& name) : ObjectBase(name) {}
		explicit PotionBase(EffectList effects) : ObjectBase(getName(effects)), _base_fx{ std::move(effects) } {}
		explicit PotionBase(const IngrList& ingredients)
		{
			const auto common{ get_common_effects(ingredients) };
			_name = getName(common);
			_base_fx = std::move(common);
		}
		explicit PotionBase(const SortedIngrList& ingredients)
		{
			IngrList ingr;
			for (auto& it : ingredients)
				ingr.push_back({ *it });
			const auto common{ get_common_effects(ingr) };
			_name = getName(common);
			_base_fx = std::move(common);
		}
	};

	/**
	 * @class Potion
	 * @brief Represents a player-made potion, and contains the expected final effects of using it.
	 */
	class Potion final : public PotionBase {
		EffectList _fx; ///< @brief The final effects of a potion, these are applied when using it.

		/**
		 * @function calculate_stats(const EffectList, GameSettings&)
		 * @brief Calculate a potion's final stats, including the players level & perks, etc.
		 * @param base	- Base effects.
		 * @param gs	- Ref of a GameSettings instance.
		 * @returns EffectList
		 */
		static EffectList calculate_stats(const EffectList& base, const GameSettings& gs)
		{
			EffectList vec;
			for ( auto& it : base )
				vec.push_back(gs.calculate(it));
			return vec;
		}

	public:
		Potion(const IngrList& ingredients, const GameSettings& gs) : PotionBase(ingredients), _fx{ calculate_stats(_base_fx, gs) } {}
		Potion(IngrList&& ingredients, const GameSettings& gs) : PotionBase(std::forward<IngrList>(ingredients)), _fx{ calculate_stats(_base_fx, gs) } {}
		Potion(SortedIngrList&& ingredients, const GameSettings& gs) : PotionBase(std::forward<SortedIngrList>(ingredients)), _fx{ calculate_stats(_base_fx, gs) } {}

		/**
		 * @function name() const
		 * @brief Retrieve the name of this potion.
		 * @returns std::string
		 */
		[[nodiscard]] std::string name() const { return _name; }
		/**
		 * @function effects() const
		 * @brief Retrieve the effects of this potion.
		 * @returns EffectList
		 */
		[[nodiscard]] EffectList effects() const { return _fx; }
		/**
		 * @function base_effects() const
		 * @brief Retrieve the base effects of this potion.
		 * @returns EffectList
		 */
		[[nodiscard]] EffectList base_effects() const { return _base_fx; }

		friend std::ostream& operator<<(std::ostream& os, const Potion& p)
		{
			os << '\t' << p._name;
			for ( auto& fx : p._fx )
				os << "\t\t" << fx._magnitude << '\t' << fx._duration << '\n';
			return os;

		}
	};
}
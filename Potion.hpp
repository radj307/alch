#pragma once
#include <unordered_set>
#include "Effect.hpp"
#include "Ingredient.hpp"
#include "GameSettings.hpp"

namespace caco_alch {
	using PotionFX = std::vector<Effect>;

	/**
	 * @function get_common_effects(const std::vector<Ingredient>&)
	 * @brief Retrieve a list of common effects with the magnitude of the strongest effect of that type. (Base Magnitude)
	 * @param ingr	- List of ingredients
	 * @returns PotionFX
	 */
	inline PotionFX get_common_effects(const std::vector<Ingredient>& ingr)
	{
		if ( ingr.size() > 4 ) throw std::exception("Too many ingredients! (Max 4)"); else if ( ingr.size() < 2 ) throw std::exception("Not enough ingredients! (Min 2)");
		PotionFX common, tmp;
		const auto is_duplicate{ [](PotionFX& target, const Effect& fx) {
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

	struct PotionBase : ObjectBase {
	protected:
		PotionFX _base_fx;

	public:
		PotionBase(const std::string& name) : ObjectBase(name), _base_fx{ } {}
		PotionBase(const std::string& name, const PotionFX& effects) : ObjectBase(name), _base_fx{ effects } {}
		PotionBase(const std::string& name, std::vector<Ingredient>&& ingredients) : ObjectBase(name), _base_fx{ get_common_effects(std::move(ingredients)) } {}
		PotionBase(const std::string& name, const std::vector<Ingredient>& ingredients) : ObjectBase(name), _base_fx{ get_common_effects(ingredients) } {}
	};

	class Potion : public PotionBase {
		PotionFX _fx;

		static PotionFX calculate_stats(const PotionFX base, const GameSettings& gs)
		{
			PotionFX vec;
			for ( auto& it : base )
				vec.push_back(Effect{ it._name, (it._magnitude > 0.0 ? gs.potion_calc_magnitude(it._magnitude) : 0.0 ), it._duration });
			return vec;
		}

	public:
		Potion(const std::string& name, std::vector<Ingredient>&& ingredients, const GameSettings& gs = _DEFAULT_GAMESETTINGS) : PotionBase(name, std::forward<std::vector<Ingredient>>(ingredients)), _fx{ calculate_stats(_base_fx, gs) } {}
		Potion(const std::string& name, const std::vector<Ingredient>& ingredients, const GameSettings& gs = _DEFAULT_GAMESETTINGS) : PotionBase(name, ingredients), _fx{ calculate_stats(_base_fx, gs) } {}
		Potion(std::vector<Ingredient>&& ingredients, const GameSettings& gs = _DEFAULT_GAMESETTINGS) : PotionBase("Potion", std::forward<std::vector<Ingredient>>(ingredients)), _fx{ calculate_stats(_base_fx, gs) } {}
		Potion(const std::vector<Ingredient>& ingredients, const GameSettings& gs = _DEFAULT_GAMESETTINGS) : PotionBase("Potion", ingredients), _fx{ calculate_stats(_base_fx, gs) } {}

		std::string name() const { return _name; }
		PotionFX effects() const { return _fx; }

		friend std::ostream& operator<<(std::ostream& os, const Potion& p)
		{
			os << '\t' << p._name;
			for ( auto& fx : p._fx )
				os << "\t\t" << fx._magnitude << '\t' << fx._duration << '\n';
			return os;

		}
	};
}
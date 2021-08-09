#pragma once
#include <unordered_set>
#include "Common.hpp"
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
		const auto is_duplicate{ [](const PotionFX& target, const Effect& fx) {
			for ( auto it{ target.begin() }; it != target.end(); ++it )
				if ( it->_name == fx._name ) // if effect names are the same, consider it a duplicate even though the magnitudes might be different
					return it;
			return target.end();
		} };
		const auto process_ingredient{ [&is_duplicate, &common, &tmp](const Ingredient& i) {
			for ( auto& it : i._effects ) {
				if ( auto dupl{ is_duplicate(tmp, it) }; dupl != tmp.end() ) { // if effect is a duplicate, push it to the common effects vector
					if ( const auto current{ is_duplicate(common, it) }; current == common.end() )
						common.push_back(*dupl); // if effect is not in the common list yet, add it
					if ( it._magnitude >= dupl->_magnitude )
						common.emplace(dupl, it); // if this effect has a higher magnitude, overwrite the current effect
				}
				else
					tmp.emplace_back(it);
			}
		} };

		for ( auto& i : ingr )
			for ( auto& it : i._effects ) {
				if ( auto dupl{ is_duplicate(tmp, it) }; dupl != tmp.end() ) { // if effect is a duplicate, push it to the common effects vector
					if ( const auto current{ is_duplicate(common, it) }; current == common.end() ) {
						if ( it._magnitude < dupl->_magnitude )
							common.push_back(*dupl); // if effect is not in the common list yet, add it
						else
							common.push_back(it);
					}
					else {
						if ( it._magnitude >= current->_magnitude ) {
							common.erase(current); // if this effect has a higher magnitude, overwrite the current effect
							common.push_back(it);
						}

					}
				}
				else
					tmp.emplace_back(it);
			}
		//	process_ingredient(it);

		return common;
	}

	struct PotionBase {
	protected:
		PotionFX _base_fx;

	public:
		PotionBase() : _base_fx{ } {}
		PotionBase(const PotionFX& effects) : _base_fx{ effects } {}
		PotionBase(std::vector<Ingredient>&& ingredients) : _base_fx{ get_common_effects(std::move(ingredients)) } {}
		PotionBase(const std::vector<Ingredient>& ingredients) : _base_fx{ get_common_effects(std::move(ingredients)) } {}
	};

	class Potion : public PotionBase {
		std::optional<std::string> _name;
		PotionFX _fx;
		size_t _dur;

		static PotionFX get_final_stats(const PotionFX base, const GameSettings& gs)
		{
			PotionFX vec;
			for ( auto& it : base )
				vec.push_back({ it._name, gs.potion_calc_magnitude(it._magnitude) });
			return vec;
		}

	public:
		Potion(const std::string& name, std::vector<Ingredient>&& ingredients, const GameSettings& gs = _internal::_DEFAULT_GAMESETTINGS) : PotionBase(std::forward<std::vector<Ingredient>>(ingredients)), _name{ name }, _fx{ get_final_stats(_base_fx, gs) }, _dur{ gs._duration } {}
		Potion(const std::string& name, const std::vector<Ingredient>& ingredients, const GameSettings& gs = _internal::_DEFAULT_GAMESETTINGS) : PotionBase(ingredients), _name{ name }, _fx{ get_final_stats(_base_fx, gs) }, _dur{ gs._duration } {}
		Potion(std::vector<Ingredient>&& ingredients, const GameSettings& gs = _internal::_DEFAULT_GAMESETTINGS) : PotionBase(std::forward<std::vector<Ingredient>>(ingredients)), _name{ std::nullopt }, _fx{ get_final_stats(_base_fx, gs) }, _dur{ gs._duration } {}
		Potion(const std::vector<Ingredient>& ingredients, const GameSettings& gs = _internal::_DEFAULT_GAMESETTINGS) : PotionBase(ingredients), _name{ std::nullopt }, _fx{ get_final_stats(_base_fx, gs) }, _dur{ gs._duration } {}

		std::string name(const std::string& value = "") const { return _name.value_or(value); }
		PotionFX effects() const { return _fx; }
		size_t duration() const { return _dur; }

		friend std::ostream& operator<<(std::ostream& os, const Potion& p)
		{
			os << '\t' << p._name.value_or("") << "\t\t( " << p._dur << "s )\n";
			for ( auto& fx : p._fx )
				os << "\t\t" << fx << '\n';
			return os;

		}
	};
}
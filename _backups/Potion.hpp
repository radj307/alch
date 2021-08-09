#pragma once
#include <set>
#include <map>
#include "Effect.hpp"
#include "Ingredient.hpp"
#include "GameSettings.hpp"

namespace caco_alch {
	/**
	 * @struct Potion
	 * @brief Represents a created potion, with estimated stats.
	 */
	struct Potion {
	private:
		GameSettings& _gs;
	public:
		std::vector<Effect> _fx;

	private:
		static std::vector<Effect> build_potion(const GameSettings& gs, std::vector<Ingredient>&& ingr)
		{
			std::map<std::string, std::set<Effect>> effects{ [&ingr]() {
				std::map<std::string, std::set<Effect>> effects;
				std::set<Effect> tmp;
				for ( auto it{ ingr.begin() }; it != ingr.end(); ++it ) { // iterate over each ingredient
					for ( auto fx{ it->_effects.begin() }; fx != it->_effects.end(); ++fx ) { // iterate over each effect
						if ( const auto ins{ tmp.insert(*fx) }; !ins.second ) {
							effects[fx->_name].insert(*ins.first);
							effects[fx->_name].insert(*fx);
						}
					}
				}
				return effects;
			}() }; ///< @brief Lambda that finds common effects in the ingredient list.
			std::vector<Effect> ret;

			ret.reserve(effects.size());
			for ( auto& [name, set] : effects )
				ret.push_back(Effect(name, gs.potion_calc_magnitude([&set]() { double mag{ 0.0 }; for ( auto& it : set ) if ( it._magnitude > mag ) mag = it._magnitude; return mag; }( ) / gs._duration)));
			ret.shrink_to_fit();

			return ret;
		}

	public:
		Potion(GameSettings& gs, std::vector<Ingredient>&& composition) : _gs{ gs }, _fx { build_potion(_gs, std::forward<std::vector<Ingredient>>(composition)) } {}

	};
}
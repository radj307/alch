#pragma once
#include <array>
#include "Effect.hpp"

namespace caco_alch {
	/**
	 * @constexpr array_match(const std::array<Effect, 4>&, const std::array<Effect, 4>&)
	 * @brief Compare two effect arrays and return true if they are a perfect match.
	 * @param a	- Comparison array A.
	 * @param b	- Comparison array B.
	 * @returns bool
	 *			true	- Effect array A and array B match.
	 *			false	- Arrays do not match.
	 */
	constexpr bool array_match(const std::array<Effect, 4>& a, const std::array<Effect, 4>& b) {
		for ( auto ia{ a.begin() }, ib{ b.begin() }; ia != a.end() && ib != b.end(); ++ia, ++ib )
			if ( ia->_name != ib->_name || ia->_magnitude != ib->_magnitude )
				return false;
		return true;
	};

	/**
	 * @struct Ingredient
	 * @brief Contains information about an ingredient, and what effects it has.
	 */
	struct Ingredient : ObjectBase {
		std::array<Effect, 4> _effects;	///< @brief This ingredient's effects.

		Ingredient() = default;
		/**
		 * @constructor Ingredient(const std::string&, const std::array<Effect, 4>&)
		 * @brief Constructor.
		 * @param name		- The name of this Ingredient.
		 * @param effects	- The effects belonging to this Ingredient.
		 */
		Ingredient(const std::string& name, const std::array<Effect, 4>& effects) : ObjectBase(name), _effects{ effects } {}

		friend std::ostream& operator<<(std::ostream& os, const Ingredient& ingr)
		{
			os << '\t' << ingr._name << '\n';
			for ( auto& fx : ingr._effects )
				os << "\t\t" << fx._name << '\t' << fx._magnitude << '\t' << fx._duration << "s\n";
			return os;
		}

		bool operator==(const Ingredient& o) const { return ( _name == o._name ) && array_match(_effects, o._effects); }
		bool operator!=(const Ingredient& o) const { return ( _name != o._name ) && !array_match(_effects, o._effects); }
		bool operator<(const Ingredient& o) {
			if ( _name.empty() || o._name.empty() ) throw std::exception("INVALID_OPERATION");
			return static_cast<short>( _name.at(0) ) < static_cast<short>( o._name.at(0) );
		}
		bool operator>(const Ingredient& o) {
			if ( _name.empty() || o._name.empty() ) throw std::exception("INVALID_OPERATION");
			return static_cast<short>( _name.at(0) ) > static_cast<short>( o._name.at(0) );
		}
	};
}
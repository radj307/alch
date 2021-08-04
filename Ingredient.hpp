#pragma once
#include <array>
#include "Effect.hpp"

/**
 * @struct Ingredient
 * @brief Contains information about an ingredient, and what effects it has.
 */
struct Ingredient {
	std::string _name;				///< @brief This ingredient's name.
	std::array<Effect, 4> _effects;	///< @brief This ingredient's effects.

	size_t _indent{ 0 };			///< @brief Used for formatting output.
	bool _quiet{ false };			///< @brief Used for formatting output.
	bool _show_magnitude{ false };	///< @brief Used for formatting output.

	Ingredient() = default;
	/**
	 * @constructor Ingredient(const std::string&, const std::array<Effect, 4>&)
	 * @brief Constructor.
	 * @param name		- The name of this Ingredient.
	 * @param effects	- The effects belonging to this Ingredient.
	 */
	Ingredient(const std::string& name, const std::array<Effect, 4>& effects) : _name{ name }, _effects{ effects } {}

	/**
	 * @function operator<<(std::ostream&, const Ingredient&)
	 * @brief Stream insertion operator.
	 * @param os	- (implicit) Output stream.
	 * @param ingr	- (implicit) Ingredient instance.
	 * @returns std::ostream&
	 */
	friend std::ostream& operator<<(std::ostream& os, const Ingredient& ingr)
	{
		const std::string tabs = std::string(ingr._indent, '\t');
		os << tabs << ingr._name << '\n';
		if ( !ingr._quiet )
			for ( auto it{ ingr._effects.begin() }; it != ingr._effects.end(); ++it ) {
				os << tabs << "  - " << it->_name;
				if ( ingr._show_magnitude )
					os << std::right << std::setw(30 - it->_name.size()) << "( " << it->_magnitude << " )";
				if ( it != ingr._effects.end() - 1 )
					os << '\n';
			}
		return os;
	}
};
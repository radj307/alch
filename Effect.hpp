#pragma once
#include <vector>
#include <string>

namespace caco_alch {
	/**
	 * @struct Effect
	 * @brief Stores information about potion effects.
	 */
	struct Effect {
		std::string _name;	///< @brief This effect's name.
		double _magnitude;	///< @brief This effect's magnitude.

		Effect() : _name{ }, _magnitude{ } {}
		/**
		 * @brief Constructor
		 * @param name	- The name of this effect.
		 */
		Effect(const std::string& name, const double magnitude) : _name{ name }, _magnitude{ magnitude } {}

		bool operator==(Effect& o) { return o._name == _name; }
		bool operator!=(Effect& o) { return o._name != _name; }
	};
}
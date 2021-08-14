#pragma once
#include <vector>
#include <string>
#include "ObjectBase.hpp"

namespace caco_alch {
	/**
	 * @struct Effect
	 * @brief Stores information about potion effects.
	 */
	struct Effect : ObjectBase {
		double _magnitude;	///< @brief This effect's magnitude.
		unsigned _duration;

		Effect() : _magnitude{ -0.0 }, _duration{ 0u } {}
		/**
		 * @brief Constructor
		 * @param name	- The name of this effect.
		 */
		Effect(const std::string& name, const double magnitude, const unsigned duration) : ObjectBase(name), _magnitude{ magnitude }, _duration{ duration } {}

		bool operator==(const Effect& o) const { return o._name == _name && o._magnitude == _magnitude; }
		bool operator!=(const Effect& o) const { return o._name != _name && o._magnitude != _magnitude; }

		bool operator<(const Effect& o) const { return _name == o._name && _magnitude < o._magnitude; }
		bool operator>(const Effect& o) const { return _name == o._name && _magnitude > o._magnitude; }
	};
}
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
		unsigned _duration;

		Effect() : _name{ }, _magnitude{ -0.0 }, _duration{ 0u } {}
		/**
		 * @brief Constructor
		 * @param name	- The name of this effect.
		 */
		Effect(const std::string& name, const double magnitude, const unsigned duration = 0) : _name{ name }, _magnitude{ magnitude }, _duration{ duration } {}

		friend std::ostream& operator<<(std::ostream& os, const Effect& fx)
		{
			os << fx._name << '\t' << fx._magnitude;
			return os;
		}

		bool operator==(const Effect& o) const { return o._name == _name && o._magnitude == _magnitude; }
		bool operator!=(const Effect& o) const { return o._name != _name && o._magnitude != _magnitude; }

		bool operator<(const Effect& o) const { return _name == o._name && _magnitude < o._magnitude; }
		bool operator>(const Effect& o) const { return _name == o._name && _magnitude > o._magnitude; }
	};
}
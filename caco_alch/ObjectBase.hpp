#pragma once
#include <string>
#include <ostream>

namespace caco_alch {
	struct ObjectBase {
		std::string _name;

		ObjectBase() = default;
		/**
		 * @constructor ObjectBase(const std::string&)
		 * @brief Default constructor for an object base.
		 * @param name - This object's name
		 */
		ObjectBase(const std::string& name) : _name{ name } {}

		bool operator<(const ObjectBase& o) const { return _name < o._name; }
		bool operator>(const ObjectBase& o) const { return _name > o._name; }
	};
}
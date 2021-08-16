#pragma once
#include <string>
#include <ostream>

namespace caco_alch {
	struct ObjectBase {
		std::string _name; ///< @brief This object's name

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
	/**
	 * @namespace _internal
	 * @brief Contains methods used internally by caco_alch.
	 */
	namespace _internal {
		/**
		 * @struct less
		 * @brief Custom implementation of the std::less comparison object to handle caco_alch's object template.
		 * @tparam T	- Type to compare
		 */
		template<class T = ObjectBase*>
		struct less {
			bool operator()(const T* a, const T* b) const { return a->_name < b->_name; } // Handle pointers
			bool operator()(const T& a, const T& b) const { return a._name < b._name; } // Handle non-pointers
		};
	}
}
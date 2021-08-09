#pragma once
#include "GameSettings.hpp"
#include "Ingredient.hpp"
namespace caco_alch {
	/**
	 * @namespace _internal
	 * @brief Contains methods used internally by the caco_alch namespace.
	 */
	namespace _internal {
		/**
		 * @struct less
		 * @brief Custom implementation of the std::less comparison object to handle caco_alch's object template.
		 * @tparam T	- Type to compare
		 */
		template<class T>
		struct less {
			bool operator()(const T* a, const T* b) const { return a->_name < b->_name; }
		};
		namespace ref {
		}

		// @brief Provides the default values for game settings.
		inline const GameSettings _DEFAULT_GAMESETTINGS{ };
	}
}
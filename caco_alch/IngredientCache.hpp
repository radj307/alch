#pragma once
#include <set>
#include <file.h>
#include "using.h"
#include "Ingredient.hpp"
#include "Potion.hpp"
#include "Format.hpp"

namespace caco_alch {

	/**
	 * @struct IngredientCache
	 * @brief Contains an ingredient list in convenient format.
	 * @tparam Sort	- The sorting predicate function used by the set.
	 * @tparam Cont	- The type of set containing the list of ingredients.
	 */
	template<template<class> class Sort = _internal::less, template<class, class> class Cont = std::set>
	struct IngredientCache {
		using Container = Cont<Ingredient, Sort<Ingredient>>;
		Container _ingr; ///< @brief This is the live cache
		const Format* _fmt{ nullptr };

		explicit IngredientCache(const Format& fmt) : _ingr{}, _fmt{ &fmt } {}
		explicit IngredientCache(Container&& ingr_cont, const Format& fmt) : _ingr{ std::move(ingr_cont) }, _fmt{ &fmt } {}
		IngredientCache(IngrList&& ingr_cont, const Format& fmt) : _ingr{ sort(std::move(ingr_cont)).first }, _fmt{ &fmt } {}

		/**
		 * @function sort(IngrList&&)
		 * @brief Sort an IngrList into a Container.
		 * @param ingr_cont	- Ingredient List rvalue.
		 * @returns std::pair<Container, unsigned>
		 *			Container	- A sorted container with all of the ingredients from ingr_cont, except duplicates.
		 *			unsigned	- The number of duplicates found.
		 */
		static std::pair<Container, unsigned> sort(IngrList&& ingr_cont)
		{
			Container list;
			unsigned count{ 0u };
			for (auto& it : ingr_cont)
				if ( const auto [existing, state]{ list.insert(it) }; !state )
					++count;
			return{ list, count };
		}

		/**
		 * @function get(const std::string&, const bool = false)
		 * @brief Retrieve an iterator to the first position in the cache matching a given name.
		 * @param name			- The name to search for
		 * @param off			- The position to begin searching at.
		 * @param only_effects	- When true, only returns ingredients with an effect matching the given search, else only returns ingredients with names matching the given search.
		 * @returns Container::iterator
		 */
		[[nodiscard]] typename Container::iterator get(const std::string& name, const typename Container::iterator& off, const bool only_effects = false)
		{
			return std::find_if(off, _ingr.end(), [&name, &only_effects](const Ingredient& i) -> bool {
				if ( only_effects )
					return std::any_of( i._effects.begin(), i._effects.end(), [&name](const Effect& fx){ return fx._name == name; } );
				return i._name == name;
			});
		}

		/**
		 * @function clear()
		 * @brief Clears the internal cache by moving it to a Container that is returned from this function.
		 * @returns Container
		 */
		Container clear() const
		{
			auto tmp{ std::move(_ingr) };
			_ingr = Container{};
			return tmp;
		}
		[[nodiscard]] IngrList getList() const
		{
			IngrList vec;
			vec.reserve(_ingr.size());
			for ( auto& it : _ingr )
				vec.emplace_back(it);
			vec.shrink_to_fit();
			return vec;
		}
		[[nodiscard]] SortedIngrList getSortedList() const
		{
			SortedIngrList vec;
			for ( auto& it : _ingr )
				vec.insert(it);
			return vec;
		}
		[[nodiscard]] bool empty() const
		{
			return _ingr.empty() || _fmt == nullptr;
		}
		IngredientCache& operator=(const Container& cont)
		{
			_ingr = cont;
			return *this;
		}
		IngredientCache& operator=(Container&& cont)
		{
			_ingr = std::move(cont);
			return *this;
		}
	};

}
#pragma once
#include "using.h"
#include "Ingredient.hpp"
#include "Format.hpp"

#include <var.hpp>

#include <set>

namespace caco_alch {

	template<typename T>
	class cache {
		using value_type = T;
		using container_type = std::vector<typename value_type>;

		container_type container;

		template<var::same_or_convertible<T>... Ts>
		cache(Ts&&... items) : container{ std::forward<Ts>(items)... } {}

		cache(container_type&& container) : container{ std::move(container) } {}
		cache(const container_type& container) : container{ container } {}

		CONSTEXPR auto begin() noexcept { return container.begin(); }
		CONSTEXPR auto end() noexcept { return container.begin(); }
		CONSTEXPR auto rbegin() noexcept { return container.rbegin(); }
		CONSTEXPR auto rend() noexcept { return container.rbegin(); }
		CONSTEXPR auto empty() const noexcept { return container.empty(); }
		CONSTEXPR auto size() const noexcept { return container.size(); }
		CONSTEXPR auto at(const size_t& idx) const noexcept { return container.at(idx); }
		CONSTEXPR auto& at(const size_t& idx) noexcept { return container.at(idx); }


		/**
		 * @brief				Retrieve an iterator to the first position in the cache matching a given name.
		 * @param name			The name to search for.
		 * @param off			The position to begin searching at.
		 * @param only_effects	When true, only returns ingredients with an effect matching the given search, else only returns ingredients with names matching the given search.
		 * @returns				container_type::const_iterator
		 */
		[[nodiscard]] typename container_type::const_iterator get(std::string name, const typename container_type::const_iterator& off, const bool only_effects = false)
		{
			name = str::tolower(name);
			const auto& pred{
				only_effects
				? [&name](Ingredient&& ingr) { return std::any_of(ingr._effects.begin(), ingr._effects.end(), [&name](Effect&& fx) { return str::tolower(fx._name) == name; }); }
			: [&name](Ingredient&& ingr) { return str::tolower(ingr._name) == name; }
			};
			return std::find_if(off, container.end(), pred);
		}

		/**
		 * @function clear()
		 * @brief Clears the internal cache by moving it to a Container that is returned from this function.
		 * @returns Container
		 */
		container_type clear() const
		{
			auto tmp{ std::move(container) };
			container = container_type{};
			return tmp;
		}
		/**
		 * @brief Copy the cache to an IngrList and return it.
		 * @returns IngrList
		 */
		[[nodiscard]] IngrList getList() const requires (std::same_as<T, Ingredient>)
		{
			return IngrList{ container };
		}
		/**
		 * @brief Copy the cache to a SortedIngrList and return it.
		 * @returns SortedIngrList
		 */
		[[nodiscard]] SortedIngrList getSortedList() const requires (std::same_as<T, Ingredient>)
		{
			SortedIngrList set;
			std::copy(container.begin(), container.end(), set.end());
			return set;
		}
	};

	/**
	 * @struct IngredientCache
	 * @brief Contains an ingredient list in convenient format.
	 * @tparam Sort	- The sorting predicate function used by the set.
	 * @tparam Cont	- The type of set containing the list of ingredients.
	 */
	template<class Cont>
	struct IngredientCache {
		using Container = Cont;
		Container _ingr; ///< @brief This is the live cache

		explicit IngredientCache() : _ingr{} {}
		explicit IngredientCache(Container&& ingr_cont) : _ingr{ std::move(ingr_cont) } {}
		IngredientCache(IngrList&& ingr_cont) : _ingr{ sort(std::move(ingr_cont)).first } {}

		IngredientCache& operator=(IngredientCache&& cache) noexcept
		{
			_ingr = std::move(cache._ingr);
			return *this;
		}
		IngredientCache& operator=(const IngredientCache& cache)
		{
			_ingr = cache._ingr;
			return *this;
		}

		auto begin() const
		{
			return _ingr.begin();
		}
		auto end() const
		{
			return _ingr.end();
		}
		auto rbegin() const
		{
			return _ingr.rbegin();
		}
		auto rend() const
		{
			return _ingr.rend();
		}

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
			Container list{};
			unsigned count{ 0u };
			for (auto& it : ingr_cont)
				if (const auto [existing, state] { list.insert(it) }; !state)
					++count;
			return{ list, count };
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
		/**
		 * @brief Copy the cache to an IngrList and return it.
		 * @returns IngrList
		 */
		[[nodiscard]] IngrList getList() const
		{
			IngrList vec;
			vec.reserve(_ingr.size());
			for (auto& it : _ingr)
				vec.emplace_back(it);
			vec.shrink_to_fit();
			return vec;
		}
		/**
		 * @brief Copy the cache to a SortedIngrList and return it.
		 * @returns SortedIngrList
		 */
		/*[[nodiscard]] SortedIngrList getSortedList() const
		{
			SortedIngrList vec;
			for (auto& it : _ingr)
				vec.insert(it);
			return vec;
		}*/
		/**
		 * @brief Check if the ingredient cache is empty.
		 * @returns bool
		 */
		[[nodiscard]] bool empty() const
		{
			return _ingr.empty();
		}
		/**
		 * @brief Set the ingredient cache by moving a given ingredient container.
		 * @param cont	- Container to move.
		 * @returns IngredientCache&
		 */
		IngredientCache& operator=(Container&& cont)
		{
			_ingr = std::move(cont);
			return *this;
		}
	};
	struct RegistryType : IngredientCache<std::set<Ingredient, _internal::less<Ingredient>>> {

		explicit RegistryType() : IngredientCache() {}
		explicit RegistryType(Container&& ingr_cont) : IngredientCache(std::move(ingr_cont)) {}
		RegistryType(IngrList&& ingr_cont) : IngredientCache(std::move(ingr_cont)) {}

		enum class FindType : unsigned char {
			BOTH,
			INGR,
			EFFECT
		};

		template<class PredT>
		Container find(std::string name, PredT&& pred, const FindType& search = FindType::BOTH) const
		{
			name = str::tolower(name);

			Container cont;

			switch (search) {
			case FindType::INGR:
				for (auto& it : _ingr) {
					if (pred(str::tolower(it._name), name))
						cont.insert(it);
				}
				break;
			case FindType::BOTH:
				for (auto& it : _ingr) {
					if (pred(str::tolower(it._name), name) || std::any_of(it._effects.begin(), it._effects.end(), [&pred, &name, this](auto&& fx) {
						return pred(str::tolower(fx._name), name);
						}))
						cont.insert(it);
				}
				break;
			case FindType::EFFECT:
				for (auto& it : _ingr)
					if (std::any_of(it._effects.begin(), it._effects.end(), [&pred, &name, this](auto&& fx) {
						return pred(str::tolower(fx._name), name);
						}))
						cont.insert(it);
			}
			return cont;
		}
		template<class PredT>
		RegistryType find_and_duplicate(std::string name, PredT&& pred, const FindType& search = FindType::BOTH) const
		{
			name = str::tolower(name);

			Container cont;

			switch (search) {
			case FindType::INGR:
				for (auto& it : _ingr)
					if (pred(str::tolower(it._name), name))
						cont.insert(it);
				break;
			case FindType::BOTH:
				for (auto& it : _ingr)
					if (pred(str::tolower(it._name), name) || std::any_of(it._effects.begin(), it._effects.end(), [&pred, &name, this](auto&& fx) { return pred(str::tolower(fx._name), name); }))
						cont.insert(it);
				break;
			case FindType::EFFECT:
				for (auto& it : _ingr)
					if (std::any_of(it._effects.begin(), it._effects.end(), [&pred, &name, this](auto&& fx) {
						return pred(str::tolower(fx._name), name);
						}))
						cont.insert(it);
			}
			return RegistryType{ std::move(cont) };
		}
		Container::const_iterator find_best_fit(std::string name, const FindType& search = FindType::INGR) const
		{
			std::vector<Container::const_iterator> vec;

			switch (search) {
			case FindType::INGR:
				for (auto it{ _ingr.begin() }; it != _ingr.end(); ++it) {
					const auto name_lc{ str::tolower(it->_name) };
					if (name_lc == name)
						return it;
					else if ((name_lc.find(name) < name_lc.size()))
						vec.emplace_back(it);
				}
				break;
			case FindType::BOTH:
				for (auto it{ _ingr.begin() }; it != _ingr.end(); ++it) {
					const auto name_lc{ str::tolower(it->_name) };
					if (name_lc == name)
						return it;
					else if ((name_lc.find(name) < name_lc.size()))
						vec.emplace_back(it);
					else for (auto fx{ it->_effects.begin() }; fx != it->_effects.end(); ++fx) {
						const auto lc{ str::tolower(fx->_name) };
						if (lc == name)
							return it;
						else if ((lc.find(name) < lc.size()))
							vec.emplace_back(it);
					}
				}
				break;
			case FindType::EFFECT:
				for (auto it{ _ingr.begin() }; it != _ingr.end(); ++it) {
					for (auto fx{ it->_effects.begin() }; fx != it->_effects.end(); ++fx) {
						const auto lc{ str::tolower(fx->_name) };
						if (lc == name)
							return it;
						else if ((lc.find(name) < lc.size()))
							vec.emplace_back(it);
					}
				}
				break;
			}
			if (vec.empty())
				return _ingr.end();
			return vec.front();
		}

		/**
		 * @brief			Find the single best ingredient according to the given predicate.
		 * @param predicate	Predicate functor where left is the current best.
		 * @returns			Container::const_iterator
		 */
		Container::const_iterator find_best(const std::function<bool(Ingredient, Ingredient)>& predicate) const
		{
			Container::const_iterator best{ _ingr.end() };
			for (auto it{ _ingr.begin() }; it != _ingr.end(); ++it)
				if (best == _ingr.end() || predicate(*best, *it))
					best = it;
			return best;
		}

		/**
		 * @brief			Retrieve a list of ingredients with a given effect in an order determined by the given sorting functor.
		 * @param fx_name	An effect name.
		 * @param sort		A sorting functor to pass to std::sort.
		 * @returns			std::vector<Ingredient>
		 */
		std::vector<Ingredient> find_best_ranked(const std::string& fx_name, const std::function<bool(Ingredient, Ingredient)>& sort) const
		{
			std::vector<Ingredient> best;
			for (auto it{ _ingr.begin() }; it != _ingr.end(); ++it)
				if (it->has_any_effect(fx_name))
					best.emplace_back(*it);
			std::sort(best.begin(), best.end(), sort);
			return best;
		}

		/**
		 * @enum	FXFindType
		 * @brief	Used by the find_best_fx functions to determine which criteria to sort by.
		*/
		enum class FXFindType : unsigned char {
			BOTH_OR,	///< @brief	Magnitude || Duration
			BOTH_AND,	///< @brief Magnitude && Duration
			MAG,		///< @brief Magnitude
			DUR			///< @brief Duration
		};

		/**
		 * @brief			Retrieve a list of ingredients with the given effect, in ascending order of that effect's magnitude and/or duration.
		 * @param fx_name	Target Effect Name.
		 * @param ft		Find Type.
		 * @returns			std::vector<Ingredient>
		 */
		Container::const_iterator find_best_fx(const std::string& fx_name, const FXFindType& ft = FXFindType::BOTH_OR, const std::vector<std::string>& excluded_ingr = {}) const
		{
			const auto excluded{ [&excluded_ingr](auto&& fx) {return std::any_of(excluded_ingr.begin(), excluded_ingr.end(), [&fx](auto&& it) {return str::tolower(fx._name) == str::tolower(it); }); } };
			const auto best{ find_best([&fx_name, &ft, &excluded](auto&& l, auto&& r) {
				const auto
					lfx{ std::find_if(l._effects.begin(), l._effects.end(), [&fx_name](auto&& fx) { return str::tolower(fx._name) == str::tolower(fx_name); }) },
					rfx{ std::find_if(r._effects.begin(), r._effects.end(), [&fx_name](auto&& fx) { return str::tolower(fx._name) == str::tolower(fx_name); }) };
				if (const auto lvalid{ lfx != l._effects.end() && !excluded(l) }, rvalid{ rfx != r._effects.end() && !excluded(r) }; lvalid && rvalid) {
					switch (ft) {
					case FXFindType::BOTH_OR:
						return (lfx->_magnitude < rfx->_magnitude) || (lfx->_duration < rfx->_duration);
					case FXFindType::BOTH_AND:
						return (lfx->_magnitude < rfx->_magnitude) && (lfx->_duration < rfx->_duration);
					case FXFindType::MAG:
						return lfx->_magnitude < rfx->_magnitude;
					case FXFindType::DUR:
						return lfx->_duration < rfx->_duration;
					default:
						return rvalid;
					}
				}
				else
					return rvalid;
				}) };
			if (best->has_any_effect(fx_name))
				return best;
			return _ingr.end();
		}
		/**
		 * @brief			Retrieve a list of ingredients with the given effect, in ascending order of that effect's magnitude and/or duration.
		 * @param fx_name	Target Effect Name.
		 * @param ft		Find Type.
		 * @returns			std::vector<Ingredient>
		 */
		auto find_best_fx_ranked(const std::string& fx_name, const FXFindType& ft = FXFindType::BOTH_OR) const
		{
			return find_best_ranked(fx_name, [&fx_name, &ft](auto&& l, auto&& r) {
				const auto
					lfx{ std::find_if(l._effects.begin(), l._effects.end(), [&fx_name](auto&& fx) { return str::tolower(fx._name) == str::tolower(fx_name); }) },
					rfx{ std::find_if(r._effects.begin(), r._effects.end(), [&fx_name](auto&& fx) { return str::tolower(fx._name) == str::tolower(fx_name); }) };
				if (const auto lvalid{ lfx != l._effects.end() }, rvalid{ rfx != r._effects.end() }; lvalid && rvalid) {
					switch (ft) {
					case FXFindType::BOTH_OR:
						return (lfx->_magnitude < rfx->_magnitude) || (lfx->_duration < rfx->_duration); // sort by magnitude OR duration
					case FXFindType::BOTH_AND:
						return (lfx->_magnitude < rfx->_magnitude) && (lfx->_duration < rfx->_duration); // sort by magnitude AND duration
					case FXFindType::MAG:
						return lfx->_magnitude < rfx->_magnitude; // sort by magnitude
					case FXFindType::DUR:
						return lfx->_duration < rfx->_duration; // sort by duration
					default:
						return lfx->_name < rfx->_name; // sort alphabetically
					}
				}
				else
					return rvalid;
			});
		}

		explicit operator Container() const { return _ingr; }
	};
}

#pragma once
#include <set>
#include <strconv.hpp>
#include <file.h>
#include "using.h"
#include "Ingredient.hpp"
#include "Potion.hpp"
#include "Format.hpp"

/**
 * @namespace caco_alch
 * @brief Contains everything used by the caco-alch project.
 */
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

	/**
	 * @class Alchemy
	 * @brief Contains the ingredient registry and functions related to interacting with it.
	 */
	class Alchemy {
	public:
		const std::exception not_found{ "NOT_FOUND" };	///< @brief Exception thrown when a given search parameter couldn't be found
		const std::exception invalid_param{ "INVALID_PARAMETERS" }; ///< @brief Exception thrown when a function receives an invalid parameter.
		using RegistryType = IngredientCache<_internal::less, std::set>;
	protected:
		const Format* _fmt{ nullptr };
		RegistryType
			_registry,
			_cache;
		const GameSettings _GMST;

	public:
#pragma region GETTERS
		/**
		 * @function is_ingr(const std::string&)
		 * @brief Check if a given ingredient exists in the ingredient list.
		 * @param name	- Name to search for.
		 * @returns bool
		 *			true  - name is a valid ingredient.
		 *			false - name is not a valid ingredient.
		 */
		[[nodiscard]] bool is_ingr(const std::string& name) { return std::any_of(_registry._ingr.begin(), _registry._ingr.end(), [&name](const IngrList::value_type& ingr) { return str::tolower(ingr._name) == name; }); }

		/**
		 * @function is_effect(const std::string&, const bool = false)
		 * @brief Check if a given ingredient exists in the ingredient list.
		 * @param name			- Name to search for.
		 * @param fuzzy_search	- When true, allows matches to contain only a part of the effect name.
		 * @returns bool
		 *			true  - name is a valid effect.
		 *			false - name is not a valid effect.
		 */
		[[nodiscard]] bool is_effect(const std::string& name, const bool fuzzy_search = false)
		{
			for ( auto& it : _registry._ingr ) {
				for ( const auto& fx : it._effects ) {
					if ( str::tolower(fx._name) == name || (fuzzy_search && str::tolower(fx._name).find(name) != std::string::npos) )
						return true;
				}
			}
			return false;
		}

		/**
		 * @function ingredients() const
		 * @brief Retrieve the list of all known ingredients.
		 * @returns IngrList
		 */
		[[nodiscard]] IngrList ingredients() const { return _registry.getList(); }

		/**
		 * @function getSortedEffectList()
		 * @brief Retrieve a sorted list of all known effects.
		 * @returns SortedEffectList
		 */
		[[nodiscard]] SortedEffectList getSortedEffectList()
		{
			SortedEffectList ret{ };
			for ( auto& i : _registry._ingr )
				for ( auto& fx : i._effects )
					ret.insert(Effect{ fx._name, -0.0, 0u });
			return ret;
		}

		/**
		 * @function find_ingr(const std::string&, const size_t = 0)
		 * @brief Retrieve a case-insensitive match from the ingredient list.
		 * @param name			- Name to search for.
		 * @param off	- Position in the list to start search from.
		 * @returns IngrList::iterator
		 */
		[[nodiscard]] SortedIngrList::iterator find_ingr(const std::string& name, const SortedIngrList::iterator& off)
		{
			return std::find_if(off, _registry._ingr.end(), [&name, this](const IngrList::value_type& ingr) -> bool
			{
				return str::tolower(ingr._name) == name || !_fmt->exact() && str::pos_valid(str::tolower(ingr._name).find(name));
			});
		}

		[[nodiscard]] SortedIngrList::iterator find_ingr(const std::string& name)
		{
			return find_ingr(name, _registry._ingr.begin());
		}

		[[nodiscard]] SortedIngrList find_ingr_list(const std::string& name)
		{
			SortedIngrList list;
			for ( auto it{ find_ingr(name) }; it != _registry._ingr.end(); it = find_ingr(name, ++it) )
				list.insert(*it);
			return list;
		}

		/**
		 * @function find_ingr_with_effect(const std::string& name)
		 * @brief Retrieve all ingredients that have a given effect.
		 * @param name			- Name to search for.
		 * @returns std::vector<Ingredient>
		 */
		[[nodiscard]] SortedIngrList find_ingr_with_effect(std::string name)
		{
			name = str::tolower(name);
			SortedIngrList set;
			for ( auto& it : _registry._ingr ) {
				for ( const auto& fx : it._effects ) {
					if ( const auto lc{ str::tolower(fx._name) }; lc == name || ( !_fmt->exact() && lc.find(name) != std::string::npos ) ) {
						set.insert(it);
						break; // from nested loop
					}
				}
			}
			return set;
		}

		/**
		 * @function find_ingr_with_effect(const std::string& name)
		 * @brief Retrieve all ingredients that have a given effect.
		 * @param names			- Names to search for.
		 * @returns std::vector<Ingredient>
		 */
		[[nodiscard]] SortedIngrList find_ingr_with_all_effects(std::vector<std::string> names)
		{
			for (auto& it : names) it = str::tolower(it);
			const auto matches{ [&names, this](const std::array<Effect, 4>& fx) {
				bool is_match{ true };
				for ( auto& name : names ) {
					if ( !std::any_of(fx.begin(), fx.end(), [&name, this](const Effect& effect) {
						return str::tolower(effect._name) == name || !_fmt->exact() && str::pos_valid(str::tolower(effect._name).find(name));
					}) )
						is_match = false;
				}
				return is_match;
			} };
			SortedIngrList set;
			for ( auto& it : _registry._ingr )
				if (matches(it._effects))
					set.insert(it);
			return set;
		}

		/**
		 * @function find_ingr_with_effect(const std::string& name)
		 * @brief Retrieve all ingredients that have a given effect.
		 * @param names			- Names to search for.
		 * @returns std::vector<Ingredient>
		 */
		[[nodiscard]] SortedIngrList find_ingr_with_any_effects(std::vector<std::string> names)
		{
			for (auto& it : names) it = str::tolower(it);
			const auto matches{ [&names, this](const std::string& lc_name) {
				return std::any_of(names.begin(), names.end(), [&lc_name, this](const std::string& name) {
					return str::tolower(name) == lc_name || !_fmt->exact() && str::pos_valid(str::tolower(name).find(lc_name));
				});
			} };
			SortedIngrList set;
			for ( auto& it : _registry._ingr ) {
				for ( const auto& fx : it._effects ) {
					if (matches(str::tolower(fx._name))) {
						set.insert(it);
						break;
					}
				}
			}
			return set;
		}
#pragma endregion GETTERS

		/**
		 * @constructor Alchemy(std::pair<EffectList, IngrList>&&)
		 * @brief Default Constructor
		 * @param ingr	- rvalue ref of loadFromFile output.
		 * @param fmt	- Format instance.
		 * @param gs	- GameSettings instance.
		 */
		explicit Alchemy(IngrList&& ingr, const Format& fmt, GameSettings&& gs) :
			_fmt{ &fmt },
			_registry{ std::move(ingr), fmt },
			_cache{ *_fmt },
			_GMST{ std::move(gs) }
		{}

		/**
		 * @function build(SortedIngrList&&, const GameSettings&)
		 * @brief Builds & returns a potion using a list of ingredients & a GameSettings instance.
		 * @param ingredients	- SortedIngrList rvalue ref.
		 * @returns Potion
		 */
		Potion build(SortedIngrList&& ingredients) const
		{
			return { std::forward<SortedIngrList>(ingredients), _GMST };
		}

		/**
		 * @function print_search_to(std::ostream&, const std::string&, const bool = true)
		 * @brief Retrieve a stringstream containing the information about the given target. Does not use cache.
		 * @param os			- Output stream to print to.
		 * @param name			- Name of the target ingredient, or target effect. This is NOT case-sensitive.
		 * @returns std::ostream&
		 */
		std::ostream& print_search_to(std::ostream& os, const std::string& name)
		{
			const auto name_lowercase{ str::tolower(name) };
			if ( SortedIngrList cont{ [&name_lowercase, this]() -> const SortedIngrList {
				if ( const auto&& tmp{ find_ingr_list(name_lowercase) }; !tmp.empty() )
					return tmp;
				if ( const auto&& tmp{ find_ingr_with_effect(name_lowercase) }; !tmp.empty() )
					return tmp;
				return {};
			}() }; !cont.empty() ) {
				os << std::fixed; // Set forced standard notation
				const auto precision{ os.precision() }; // copy current output stream precision
				os.precision(_fmt->precision()); // Set floating-point-precision.
				if ( _fmt->file_export() ) // export registry-format ingredients
					_fmt->to_fstream(os, cont);
				else { // insert search results
					os << Color::f::green << "Search results for: \'" << Color::f::yellow << name << Color::f::green << "\'\n" << Color::f::red << "{\n" << Color::reset;
					if ( _fmt->reverse_output() )
						for ( auto it{ cont.rbegin() }; it != cont.rend(); ++it )
							_fmt->to_stream(os, *it, name_lowercase);
					else
						for ( auto it{ cont.begin() }; it != cont.end(); ++it )
							_fmt->to_stream(os, *it, name_lowercase);
					os << Color::f::red << "}" << Color::reset << std::endl;
				}
				os.precision(precision); // reset output stream precision
			}
			else // no results found
				os << sys::error << "Didn't find any ingredients or effects matching \'" << Color::f::yellow << name << Color::reset << "\'\n";
			return os;
		}

		/**
		 * @function print_search_to(std::ostream&, const std::string&, const bool = true)
		 * @brief Retrieve a stringstream containing the information about the given target.
		 * @param os			- Output stream to print to.
		 * @param names			- Names of the target effects. This is NOT case-sensitive.
		 * @returns std::ostream&
		 */
		std::ostream& print_smart_search_to(std::ostream& os, std::vector<std::string> names)
		{
			os << Color::f::green << "Search results for ";
			for ( auto it{ names.begin() }; it != names.end(); ++it ) {
				*it = str::tolower(*it);
				os << Color::f::green << '\'' << Color::f::yellow << *it << Color::f::green << '\'';
				const auto has_next{ it + 1 != names.end() };
				if ( has_next && it + 2 == names.end() )
					os << " and ";
				else if ( has_next )
					os << ", ";
			}
			os << Color::reset << '\n' << Color::f::red << "{\n" << Color::reset;

			RegistryType cache{ *_fmt };

			bool is_cache{ false };
			for ( auto name{ names.begin() }; name != names.end(); ++name ) {
				if ( !is_cache ) { // if cache hasn't been populated yet
					is_cache = true;
					if ( !_cache.empty() )
						cache = _cache; // currently there is no way to use the long-term cache
					else
						cache = find_ingr_with_effect(*name);
				}
				else {
					decltype(cache._ingr) tmp{  };
					for ( auto& it : cache._ingr )
						if ( std::any_of(it._effects.begin(), it._effects.end(), [&name, this](const Effect& fx){ const auto lc{ str::tolower(fx._name) }; return lc == *name || !_fmt->exact() && str::pos_valid(lc.find(*name)); }) )
							tmp.insert(it);
					cache._ingr = tmp;
				}
				if ( name + 1 == names.end() ) {
					os << std::fixed; // Set forced standard notation
					const auto precision{ os.precision() }; // copy current output stream precision
					os.precision(_fmt->precision()); // Set floating-point-precision.
					if ( _fmt->file_export() ) // export registry-format ingredients
						_fmt->to_fstream(os, cache._ingr);
					else
						_fmt->to_stream(os, cache._ingr, names);
					os.precision(precision); // reset output stream precision
				}
				if ( is_cache && cache.empty() ) {
					os << sys::error << "Didn't find anything after applying filter for \'" << Color::f::yellow << *name << Color::reset << "\'\n";
					break;
				}
			}
			os << Color::f::red << "}\n" << Color::reset;
			_cache = std::move(cache);
			return os;
		}

		/**
		 * @function print_list_to(std::ostream&, const bool, const bool = false, const bool = false, const size_t = 3)
		 * @brief Insert the ingredient list into an output stream.
		 * @param os				- Target Output Stream.
		 * @returns std::ostream&
		 */
		std::ostream& print_list_to(std::ostream& os) const
		{
			if ( !_registry._ingr.empty() ) {
				os << std::fixed; // force standard notation
				const auto precision{ os.precision() }; // copy the current precision
				os.precision(_fmt->precision()); // set precision to 2 decimal places

				if ( _fmt->file_export() )
					_fmt->to_fstream(os, _registry._ingr);
				else {
					os << Color::f::green << "Ingredients" << Color::reset << '\n' << Color::f::red << '{' << Color::reset << '\n';

					os << Color::f::red << '}' << Color::reset << '\n';
				}

				os.precision(precision); // reset precision
			}
			return os;
		}

		/**
		 * @function print_build_to(std::ostream&, IngrList&&, const GameSettings&, const Format& = {})
		 * @brief Construct a potion with the specified ingredients, and output the result to os.
		 * @param os	- Target output stream
		 * @param cont	- List of ingredients ( Min. 2 <= x <= Max. 4 ). If more than 4 ingredients are passed in, the first 4 are used and the rest are discarded.
		 * @param max4	- Whether to enforce the 4 ingredient limit or not.
		 * @returns std::ostream&
		 */
		std::ostream& print_build_to(std::ostream& os, const IngrList& cont, const bool max4 = true) const
		{
			SortedIngrList set{};
			for ( auto& it : cont )
				set.insert(it);
			if ( set.size() >= 2 ) { // if at least 2 valid ingredients were found
				if ( max4 && set.size() > 4 )
					set = [](const SortedIngrList& set){ SortedIngrList nset; unsigned count{0u}; for (auto& it : set) { nset.insert(it); if (++count == 4u) break; } return nset; }(set);
				const auto indentation{ std::string(_fmt->indent(), ' ') };
				const auto precision{ os.precision() };
				os.precision(_fmt->precision());
				os << std::fixed;

				const auto skill_base{ _GMST.fAlchemyAV() };
				os << Color::f::green << "Potion Builder [Alchemy Skill: " << Color::f::cyan << skill_base;
				if ( const auto skill_mod{ _GMST.fAlchemyMod() }; skill_mod > 0.0 )
					os << Color::reset << "(" << Color::f::green << skill_base + skill_mod << Color::reset << ")";
				os << Color::f::green << ']' << Color::reset << '\n';

				const auto potion{ build(std::forward<SortedIngrList>(set)) };

				os << Color::f::green << "Input:\n" << Color::f::red << '{' << Color::reset << '\n';
				for ( auto& it : set )
					_fmt->to_stream_build(os, it, potion);
				os << Color::f::red << '}' << Color::reset << '\n';

				if ( potion.effects().empty() )
					throw std::exception("Potion creation failed.");

				os << Color::f::green << "Output:\n" << Color::f::red << '{' << Color::reset << '\n';

				_fmt->to_stream(os, potion, indentation);

				os << Color::f::red << '}' << Color::reset << '\n';
				os.precision(precision); // Reset precision
				return os;
			}
			throw invalid_param;
		}

		/**
		 * @function print_build_to(std::ostream&, const std::vector<std::string>&, const GameSettings&, const Format& = {})
		 * @brief Construct a potion with the specified ingredients, and output the result to os.
		 * @param os	- Target output stream
		 * @param names	- List of ingredient names ( Min. 2 <= x <= Max. 4 ). If more than 4 ingredients are passed in, the first 4 are used and the rest are discarded.
		 * @returns std::ostream&
		 */
		std::ostream& print_build_to(std::ostream& os, const std::vector<std::string>& names)
		{
			if ( names.size() > 4 ) throw std::exception("Too many ingredients! (Build Mode Max 4)"); else if ( names.size() < 2 ) throw std::exception("Not enough ingredients! (Build Mode Min 2)");
			std::vector<Ingredient> cont;
			for ( auto& it : names ) {
				const auto ingr{ find_ingr(str::tolower(it)) };
				if ( ingr != _registry._ingr.end() )
					cont.push_back(*ingr);
				else os << sys::warn << "Couldn't find ingredient: \"" << it << "\"\n";
			}
			if ( cont.size() > 4 ) cont.erase(cont.begin() + 4u, cont.end());
			return print_build_to(os, std::forward<std::vector<Ingredient>>(cont), _fmt);
		}
		[[nodiscard]] bool writeCacheToFile(const std::string& filename, const bool append = false) const
		{
			if ( !_cache.empty() )
				return file::write(filename, std::stringstream{ _fmt->to_fstream(_cache._ingr) }, append);
			return false;
		}
	};
}
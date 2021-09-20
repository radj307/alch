#pragma once
#include <set>
#include <TermAPI.hpp>
#include <strconv.hpp>
#include <strmanip.hpp>
#include <file.h>
#include "using.h"
#include "Ingredient.hpp"
#include "Potion.hpp"
#include "Format.hpp"
#include "IngredientCache.hpp"

/**
 * @namespace caco_alch
 * @brief Contains everything used by the caco-alch project.
 */
namespace caco_alch {
	/**
	 * @class Alchemy
	 * @brief Contains the ingredient registry and functions related to interacting with it.
	 */
	class Alchemy {
	protected:
		const Format _fmt;
		RegistryType
			_registry,
			_cache;
		const GameConfig _GMST;

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
#pragma endregion GETTERS

		/**
		 * @constructor Alchemy(std::pair<EffectList, IngrList>&&)
		 * @brief Default Constructor
		 * @param ingr	- rvalue ref of loadFromFile output.
		 * @param fmt	- Format instance.
		 * @param gs	- GameConfig instance.
		 */
		explicit Alchemy(IngrList&& ingr, const Format& fmt, const GameConfig& gs) :
			_fmt{ fmt },
			_registry{ std::move(ingr), fmt },
			_cache{ _fmt },
			_GMST{ gs }
		{}

		/**
		 * @function build(SortedIngrList&&, const GameConfig&)
		 * @brief Builds & returns a potion using a list of ingredients & a GameConfig instance.
		 * @param ingredients	- SortedIngrList rvalue ref.
		 * @returns Potion
		 */
		Potion build(const SortedIngrList& ingredients) const
		{
			return Potion{ ingredients, _GMST };
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
			if ( SortedIngrList cont{ _registry.find(name) }; !cont.empty() ) {
				os << std::fixed; // Set forced standard notation
				const auto precision{ os.precision() }; // copy current output stream precision
				os.precision(_fmt.precision()); // Set floating-point-precision.
				if ( _fmt.file_export() ) // export registry-format ingredients
					_fmt.to_fstream(os, cont);
				else { // insert search results
					os << _fmt._colors.set(UIElement::SEARCH_HEADER) << "Search results for: \'" << _fmt._colors.set(UIElement::SEARCH_HIGHLIGHT) << name << color::reset << _fmt._colors.set(UIElement::SEARCH_HEADER) << "\'\n" << _fmt._colors.set(UIElement::BRACKET) << "{\n" << color::reset;
					if ( _fmt.reverse_output() )
						for ( auto it{ cont.rbegin() }; it != cont.rend(); ++it )
							_fmt.to_stream(os, *it, name_lowercase);
					else
						for ( auto it{ cont.begin() }; it != cont.end(); ++it )
							_fmt.to_stream(os, *it, name_lowercase);
					os << _fmt._colors.set(UIElement::BRACKET) << "}" << color::reset << std::endl;
				}
				os.precision(precision); // reset output stream precision
			}
			else // no results found
				os << sys::term::error << "Didn't find any ingredients or effects matching \'" << _fmt._colors.set(UIElement::SEARCH_HIGHLIGHT) << name << color::reset << "\'\n";
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
			os << _fmt._colors.set(UIElement::SEARCH_HEADER) << "Search results for " << color::reset;
			for ( auto it{ names.begin() }; it != names.end(); ++it ) {
				*it = str::tolower(*it);
				os << _fmt._colors.set(UIElement::SEARCH_HEADER) << '\'' << color::reset << _fmt._colors.set(UIElement::SEARCH_HIGHLIGHT) << *it << color::reset << _fmt._colors.set(UIElement::SEARCH_HEADER) << '\'';
				const auto has_next{ it + 1 != names.end() };
				if ( has_next && it + 2 == names.end() )
					os << " and ";
				else if ( has_next )
					os << ", ";
			}
			os << color::reset << '\n' << _fmt._colors.set(UIElement::BRACKET) << "{\n" << color::reset;

			RegistryType cache{ _fmt };

			bool is_cache{ false };
			for ( auto name{ names.begin() }; name != names.end(); ++name ) {
				if ( !is_cache ) { // if cache hasn't been populated yet
					is_cache = true;
					if (!_cache.empty())
						cache = _cache; // currently there is no way to use the long-term cache
					else
						cache = std::move(_registry.find_and_duplicate(*name, RegistryType::FindType::EFFECT));
				}
				else {
					decltype(cache._ingr) tmp{  };
					for ( auto& it : cache._ingr )
						if ( std::any_of(it._effects.begin(), it._effects.end(), [&name, this](const Effect& fx){ const auto lc{ str::tolower(fx._name) }; return lc == *name || !_fmt._flag_exact && str::pos_valid(lc.find(*name)); }) )
							tmp.insert(it);
					cache._ingr = tmp;
				}
				if ( name + 1 == names.end() ) {
					os << std::fixed; // Set forced standard notation
					const auto precision{ os.precision() }; // copy current output stream precision
					os.precision(_fmt.precision()); // Set floating-point-precision.
					if ( _fmt.file_export() ) // export registry-format ingredients
						_fmt.to_fstream(os, cache._ingr);
					else
						_fmt.to_stream(os, cache._ingr, names);
					os.precision(precision); // reset output stream precision
				}
				if ( is_cache && cache.empty() ) {
					os << sys::term::error << "Didn't find anything after applying filter for \'" << _fmt._colors.set(UIElement::SEARCH_HIGHLIGHT) << *name << color::reset << "\'\n";
					break;
				}
			}
			os << _fmt._colors.set(UIElement::BRACKET) << "}\n" << color::reset;
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
				os.precision(_fmt.precision()); // set precision to 2 decimal places

				if ( _fmt.file_export() )
					_fmt.to_fstream(os, _registry._ingr);
				else {
					os << _fmt._colors.set(UIElement::SEARCH_HEADER) << "Ingredients" << color::reset << '\n' << _fmt._colors.set(UIElement::BRACKET) << '{' << color::reset << '\n';
					_fmt.list_to_stream(os, _registry.getSortedList());
					os << _fmt._colors.set(UIElement::BRACKET) << '}' << color::reset << '\n';
				}

				os.precision(precision); // reset precision
			}
			return os;
		}

		/**
		 * @function print_build_to(std::ostream&, IngrList&&, const GameConfig&, const Format& = {})
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
				const auto indentation{ std::string(_fmt.indent(), ' ') };
				const auto precision{ os.precision() };
				os.precision(_fmt.precision());
				os << std::fixed;

				const auto skill_base{ _GMST.fAlchemyAV() };
				os << _fmt._colors.set(UIElement::SEARCH_HEADER) << "Potion Builder [Alchemy Skill: " << _fmt._colors.set(UIElement::ALCHEMY_SKILL) << skill_base << color::reset;
				if ( const auto skill_mod{ _GMST.fAlchemyMod() }; skill_mod > 0.0 )
					os << "(" << _fmt._colors.set(UIElement::ALCHEMY_SKILL) << skill_base + skill_mod << color::reset << ")";
				os << _fmt._colors.set(UIElement::SEARCH_HEADER) << ']' << color::reset << '\n';

				const auto potion{ build(set) };

				os << _fmt._colors.set(UIElement::SEARCH_HEADER) << "Input:\n" << _fmt._colors.set(UIElement::BRACKET) << '{' << color::reset << '\n';
				for ( auto& it : set )
					_fmt.to_stream_build(os, it, potion);
				os << _fmt._colors.set(UIElement::BRACKET) << '}' << color::reset << '\n';

				if ( potion.effects().empty() )
					throw std::exception("Potion creation failed.");

				os << _fmt._colors.set(UIElement::SEARCH_HEADER) << "Output:\n" << _fmt._colors.set(UIElement::BRACKET) << '{' << color::reset << '\n';

				_fmt.to_stream(os, potion, indentation);

				os << _fmt._colors.set(UIElement::BRACKET) << '}' << color::reset << '\n';
				os.precision(precision); // Reset precision
				return os;
			}
			throw std::exception(std::string("Build failed! Try including more than one ingredient.").c_str());
		}

		/**
		 * @function print_build_to(std::ostream&, const std::vector<std::string>&, const GameConfig&, const Format& = {})
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
				const auto ingr{ _registry.find_best_fit(str::tolower(it)) };
				if ( ingr != _registry._ingr.end() )
					cont.push_back(*ingr);
				else os << sys::term::warn << "Couldn't find ingredient: \"" << it << "\"\n";
			}
			if ( cont.size() > 4 ) cont.erase(cont.begin() + 4u, cont.end());
			return print_build_to(os, std::forward<std::vector<Ingredient>>(cont));
		}
		[[nodiscard]] bool writeCacheToFile(const std::string& filename, const bool append = false) const
		{
			if ( !_cache.empty() )
				return file::write(filename, std::stringstream{ _fmt.to_fstream(_cache._ingr) }, append);
			return false;
		}
	};
}
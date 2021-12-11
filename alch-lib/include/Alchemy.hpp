#pragma once
#include "using.h"
#include "Ingredient.hpp"
#include "Potion.hpp"
#include "Format.hpp"
#include "IngredientCache.hpp"

#include <str.hpp>
#include <TermAPI.hpp>
#include <fileio.hpp>

#include <set>

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
			for (auto& it : _registry._ingr) {
				for (const auto& fx : it._effects) {
					if (str::tolower(fx._name) == name || (fuzzy_search && str::tolower(fx._name).find(name) != std::string::npos))
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
			for (auto& i : _registry._ingr)
				for (auto& fx : i._effects)
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
		 * @brief Retrieve a stringstream containing the information about the given target. Does not use cache.
		 * @param os			- Output stream to print to.
		 * @param name			- Name of the target ingredient, or target effect. This is NOT case-sensitive.
		 * @returns std::ostream&
		 */
		std::ostream& print_search(std::ostream& os, const std::string& name) const
		{
			if (const auto results{ _registry.find(name) }; !results.empty()) {
				const auto precision{ os.precision() };
				os.precision(_fmt._precision);
				os << std::fixed << ColorAPI.set(UIElement::SEARCH_HEADER) << "Search results for: \"" << color::reset << ColorAPI.set(UIElement::SEARCH_HIGHLIGHT) << name << color::reset << ColorAPI.set(UIElement::SEARCH_HEADER) << '\"' << color::reset << '\n' << ColorAPI.set(UIElement::BRACKET) << '{' << color::reset << '\n' << _fmt.print(results, std::vector<std::string>{ name }) << '\n' << ColorAPI.set(UIElement::BRACKET) << '}' << color::reset << '\n';
				os.precision(precision);
			}
			else
				os << sys::term::error << "Didn't find any ingredients or effects matching \"" << ColorAPI.set(UIElement::SEARCH_HIGHLIGHT) << name << color::reset << "\"\n";
			return os;
		}

		/**
		 * @brief Retrieve a stringstream containing the information about the given target.
		 * @param os			- Output stream to print to.
		 * @param names			- Names of the target effects. This is NOT case-sensitive.
		 * @returns std::ostream&
		 */
		std::ostream& print_smart_search(std::ostream& os, std::vector<std::string> names) const
		{
			RegistryType cache{ _fmt };
			bool init{ false };
			os << ColorAPI.set(UIElement::SEARCH_HEADER) << "Search results for " << color::reset;
			for (auto name{ names.begin() }; name != names.end(); ++name) {
				os << ColorAPI.set(UIElement::SEARCH_HEADER) << '\"' << color::reset << ColorAPI.set(UIElement::SEARCH_HIGHLIGHT) << *name << color::reset << ColorAPI.set(UIElement::SEARCH_HEADER) << '\"';
				const auto has_next{ name + 1 != names.end() };
				if (has_next && name + 2 == names.end())
					os << " and ";
				else if (has_next)
					os << ", ";
			}
			os << '\n' << ColorAPI.set(UIElement::BRACKET) << '{' << color::reset << '\n';
			for (auto name{ names.begin() }; name != names.end(); ++name) {
				if (!init) {
					cache = std::move(_registry.find_and_duplicate(*name, RegistryType::FindType::EFFECT));
					init = true;
				}
				else {
					decltype(cache._ingr) tmp{};
					for (auto& it : cache._ingr) {
						if (std::any_of(it._effects.begin(), it._effects.end(), [&name, this](const Effect& fx) {
							const auto lc{ str::tolower(fx._name) };
							return lc == *name || !_fmt._flag_exact && str::pos_valid(lc.find(*name));
							}
							)) {
							tmp.insert(it);
						}
					}
					cache._ingr = tmp;
				}
				if (name + 1 == names.end()) {
					os << std::fixed;
					const auto precision{ os.precision() };
					os.precision(_fmt._precision);
					os << std::fixed << _fmt.print(cache._ingr, names);
					os.precision(precision);
				}
				if (init && cache.empty()) {
					os << sys::term::error << "Didn't find anything after applying filter for \"" << ColorAPI.set(UIElement::SEARCH_HIGHLIGHT) << *name << color::reset << "\"\n";
					break;
				}
			}
			os << '\n' << ColorAPI.set(UIElement::BRACKET) << '}' << color::reset << '\n';
			return os;
		}

		/**
		 * @brief Insert the ingredient list into an output stream.
		 * @param os				- Target Output Stream.
		 * @returns std::ostream&
		 */
		std::ostream& print_list(std::ostream& os) const
		{
			os << std::fixed;
			const auto precision{ os.precision() };
			os.precision(_fmt._precision);
			os << ColorAPI.set(UIElement::SEARCH_HEADER) << "Ingredients" << color::reset << '\n' << ColorAPI.set(UIElement::BRACKET) << '{' << color::reset << '\n' << _fmt.print(_registry._ingr, std::nullopt) << '\n' << ColorAPI.set(UIElement::BRACKET) << '}' << color::reset << '\n';
			os.precision(precision);
			return os;
		}
		/**
		 * @brief Build a potion and print it the given output stream, using a set of ingredients.
		 * @param os	- Target output stream.
		 * @param ingr	- Ingredients to use.
		 * @returns std::ostream&
		 */
		std::ostream& print_build(std::ostream& os, SortedIngrList ingr) const
		{
			if (ingr.size() >= 2) {
				os << std::fixed;
				const auto precision{ os.precision() };
				os.precision(_fmt._precision); // set precision

				const auto skill_base{ _GMST.fAlchemyAV() };
				const auto skill_mod{ _GMST.fAlchemyMod() };
				os << ColorAPI.set(UIElement::SEARCH_HEADER) << "Potion Builder [Alchemy Skill: " << color::reset << ColorAPI.set(UIElement::ALCHEMY_SKILL) << skill_base;
				if (skill_mod > 0.0)
					os << '(' << static_cast<unsigned>(std::round(static_cast<double>(skill_base) + skill_mod)) << ')';
				os << color::reset << "]\n";
				// Print input
				os << ColorAPI.set(UIElement::SEARCH_HEADER) << "Input:" << color::reset << '\n' << ColorAPI.set(UIElement::BRACKET) << '{' << color::reset << '\n' << _fmt.print(ingr) << '\n' << ColorAPI.set(UIElement::BRACKET) << '}' << color::reset << '\n';
				// Print output

				if (const auto potion{ build(ingr) }; potion.effects().empty())
					throw std::exception("Potion Creation Failed.");
				else
					os << ColorAPI.set(UIElement::SEARCH_HEADER) << "Output:" << color::reset << '\n' << ColorAPI.set(UIElement::BRACKET) << '{' << color::reset << '\n' << _fmt.print(potion) << '\n' << ColorAPI.set(UIElement::BRACKET) << '}' << color::reset << '\n';
				os.precision(precision); // reset precision
			}
			else
				throw std::exception("Build failed! At least two valid ingredients must be specified.");
			return os;
		}
		/**
		 * @brief Build a potion and print it to the given output stream, using a vector of strings to search for.
		 * @param os	- Target output stream.
		 * @param names	- Ingredient names to search for.
		 * @returns std::ostream&
		 */
		std::ostream& print_build(std::ostream& os, std::vector<std::string> names, const unsigned max = 4) const
		{
			SortedIngrList ingr;
			// search for ingredient names
			for (auto& name : names) {
				if (const auto it{ _registry.find_best_fit(name, RegistryType::FindType::INGR) }; it != _registry.end())
					ingr.insert(*it);
				if (ingr.size() >= max)
					break;
			}
			return print_build(os, std::move(ingr));
		}
		/**
		 * @brief Build a potion and print it to the given output stream, using a vector of ingredients.
		 * @param os	- Target output stream.
		 * @param ingr	- Ingredients to use.
		 * @returns std::ostream&
		 */
		std::ostream& print_build(std::ostream& os, const IngrList& ingr) const
		{
			SortedIngrList set;
			for (auto& it : ingr)
				set.insert(it);
			return print_build(os, std::move(set));
		}
	};
}
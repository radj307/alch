#pragma once
#include <set>
#include <strconv.hpp>
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
	 * @class Alchemy
	 * @brief Contains the ingredient registry and functions related to interacting with it.
	 */
	class Alchemy {
	public:
		const std::exception not_found{ "NOT_FOUND" };	///< @brief Exception thrown when a given search parameter couldn't be found
		const std::exception invalid_param{ "INVALID_PARAMETERS" }; ///< @brief Exception thrown when a function receives an invalid parameter.
	protected:
		IngrList _ingr; ///< @brief Stores the list of all known ingredients.

	public:
		/**
		 * @function is_ingr(const std::string&)
		 * @brief Check if a given ingredient exists in the ingredient list.
		 * @param name	- Name to search for.
		 * @returns bool
		 *			true  - name is a valid ingredient.
		 *			false - name is not a valid ingredient.
		 */
		bool is_ingr(const std::string& name) { return std::any_of(_ingr.begin(), _ingr.end(), [&name](const IngrList::value_type& ingr) { return str::tolower(ingr._name) == name; }); }

		/**
		 * @function is_effect(const std::string&, const bool = false)
		 * @brief Check if a given ingredient exists in the ingredient list.
		 * @param name			- Name to search for.
		 * @param fuzzy_search	- When true, allows matches to contain only a part of the effect name.
		 * @returns bool
		 *			true  - name is a valid effect.
		 *			false - name is not a valid effect.
		 */
		bool is_effect(const std::string& name, const bool fuzzy_search = false)
		{
			for ( auto& it : _ingr ) {
				for ( const auto& fx : it._effects ) {
					if ( str::tolower(fx._name) == name || (fuzzy_search && str::tolower(fx._name).find(name) != std::string::npos) )
						return true;
				}
			}
			return false;
		}

		/**
		 * @function get_ingr_ref()
		 * @brief Retrieve a reference to the ingredient list.
		 * @returns IngrList&
		 */
		IngrList& get_ingr_ref() { return _ingr; }

		/**
		 * @function getSortedEffectList()
		 * @brief Retrieve a sorted list of all known effects.
		 * @returns SortedEffectList
		 */
		SortedEffectList getSortedEffectList()
		{
			SortedEffectList ret{ };
			for ( auto& i : _ingr )
				for ( auto& fx : i._effects )
					ret.insert(Effect{ fx._name, static_cast<double>(-0.0), static_cast<unsigned>(0) });
			return ret;
		}

		/**
		 * @function find_ingr(const std::string&, const size_t = 0)
		 * @brief Retrieve a case-insensitive match from the ingredient list.
		 * @param name			- Name to search for.
		 * @param fuzzy_search	- When true, allows partial matches in the returned list. (Ex: "smith" -> "fortify smithing")
		 * @param off	- Position in the list to start search from.
		 * @returns IngrList::iterator
		 */
		IngrList::iterator find_ingr(const std::string& name, const bool fuzzy_search, const int off = 0)
		{
			return std::find_if(_ingr.begin() + off, _ingr.end(), [&name](const IngrList::value_type& ingr) -> bool { return str::tolower(ingr._name) == name; });
		}
		/**
		 * @function get_ingr_list(const std::string&)
		 * @brief Retrieve all ingredients that have a given string in their name.
		 * @param name			- Name to search for.
		 * @param fuzzy_search	- When true, allows partial matches in the returned list. (Ex: "smith" -> "fortify smithing")
		 * @returns std::vector<Ingredient>
		 */
		SortedIngrList get_ingr_list(const std::string& name, const bool fuzzy_search)
		{
			SortedIngrList set;
			for ( auto& it : _ingr )
				if ( str::tolower(it._name) == name || ( fuzzy_search && str::tolower(it._name).find(name) != std::string::npos ) )
					set.insert(&it);
			return set;
		}
		/**
		 * @function find_ingr_with_effect(const std::string& name)
		 * @brief Retrieve all ingredients that have a given effect.
		 * @param name			- Name to search for.
		 * @param fuzzy_search	- When true, allows partial matches in the returned list. (Ex: "smith" -> "fortify smithing")
		 * @returns std::vector<Ingredient>
		 */
		SortedIngrList find_ingr_with_effect(std::string name, const bool fuzzy_search)
		{
			name = str::tolower(name);
			SortedIngrList set;
			for ( auto& it : _ingr ) {
				for ( const auto& fx : it._effects ) {
					if ( const auto lc{ str::tolower(fx._name) }; lc == name || ( fuzzy_search && lc.find(name) != std::string::npos ) ) {
						set.insert(&it);
						break; // from nested loop
					}
				}
			}
			return set;
		}

		/**
		 * @constructor Alchemy(std::pair<EffectList, IngrList>&&)
		 * @brief Default Constructor
		 * @param ingr	- rvalue ref of loadFromFile output.
		 */
		explicit Alchemy(IngrList&& ingr) : _ingr{ std::move(ingr) } {}

		/**
		 * @function print_search_to(std::ostream&, const std::string&, const bool = false, const bool = false, const size_t = 3)
		 * @brief Retrieve a stringstream containing the information about the given target.
		 * @param os			- Output stream to print to.
		 * @param name			- Name of the target ingredient, or target effect. This is NOT case-sensitive.
		 * @param fmt			- Format instance, used to format the text output.
		 * @returns std::ostream&
		 */
		std::ostream& print_search_to(std::ostream& os, const std::string& name, const Format& fmt = Format{ })
		{
			const auto name_lowercase{ str::tolower(name) };
			if ( SortedIngrList cont{ [&fmt, &name_lowercase, this]() -> const SortedIngrList {
				if ( const auto&& tmp{ find_ingr_with_effect(name_lowercase, !fmt.exact()) }; !tmp.empty() )
					return tmp;
				if ( const auto&& tmp{ find_ingr_with_effect(name_lowercase, !fmt.exact()) }; !tmp.empty() )
					return tmp;
				return {};
			}() }; !cont.empty() ) {
				os << std::fixed; // Set forced standard notation
				const auto precision{ os.precision() }; // copy current output stream precision
				os.precision(fmt.precision()); // Set floating-point-precision.
				if ( fmt.file_export() ) // export registry-format ingredients
					fmt.to_fstream(os, cont);
				else { // insert search results
					os << Color::f_green << "Search results for: \'" << Color::f_yellow << name << Color::f_green << "\'\n" << Color::f_red << "{\n" << Color::reset;
					if ( fmt.reverse_output() )
						for ( auto it{ cont.rbegin() }; it != cont.rend(); ++it )
							fmt.to_stream(os, **it, name_lowercase);
					else
						for ( auto it{ cont.begin() }; it != cont.end(); ++it )
							fmt.to_stream(os, **it, name_lowercase);
					os << Color::f_red << "}" << Color::reset << std::endl;
				}
				os.precision(precision); // reset output stream precision
			}
			else // no results found
				os << sys::error << "Didn't find any ingredients or effects matching \'" << Color::f_yellow << name << Color::reset << "\'\n";
			return os;
		}

		/**
		 * @function print_list_to(std::ostream&, const bool, const bool = false, const bool = false, const size_t = 3)
		 * @brief Insert the ingredient list into an output stream.
		 * @param os				- Target Output Stream.
		 * @param fmt				- Format instance, used to format the text output.
		 * @returns std::ostream&
		 */
		std::ostream& print_list_to(std::ostream& os, const Format& fmt = Format{ })
		{
			SortedIngrList set;
			for ( auto it{ _ingr.begin() }; it != _ingr.end(); ++it )
				set.insert(&*it);
			if ( !set.empty() ) {
				os << std::fixed; // force standard notation
				const auto precision{ os.precision() }; // copy the current precision
				os.precision(fmt.precision()); // set precision to 2 decimal places

				if ( fmt.file_export() )
					fmt.to_fstream(os, set);
				else {
					os << Color::f_green << "Ingredients" << Color::reset << '\n' << Color::f_red << '{' << Color::reset << '\n';
					if ( fmt.reverse_output() )
						for ( auto it{ set.rbegin() }; it != set.rend(); ++it )
							fmt.to_stream(os, **it);
					else
						for ( auto it{ set.begin() }; it != set.end(); ++it )
							fmt.to_stream(os, **it);
					os << Color::f_red << '}' << Color::reset << '\n';
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
		 * @param gs	- GameSettings instance, provides the stats necessary for simulating the alchemy algorithm.
		 * @param fmt	- Alchemy Format object, controls how the results appear in the output stream.
		 * @returns std::ostream&
		 */
		std::ostream& print_build_to(std::ostream& os, IngrList cont, const GameSettings& gs, const Format& fmt = Format{ }) const
		{
			const auto verbose{ [&os, &fmt](Effect& fx) {
				if ( ( fx._magnitude != 0.0 && ( fmt.verbose() || !fmt.quiet() ) ) || fmt.all() ) {
					if ( fx._name.size() > 30u ) // name is long
						os << std::setw(fx._name.size()) << ' ';
					else // name is normal length
						os << std::setw(30 - fx._name.size()) << ' ';
					const auto mag_str{ [&fx, &fmt]() -> std::string { auto str{ std::to_string(fx._magnitude) }; if ( const auto dPos{ str.find('.') }; str::pos_valid(dPos) ) str = str.substr(0u, dPos + fmt.precision() + 1); return str; }( ) };
					os << Color::f_gray << mag_str << std::setw(mag_str.size() < 16 ? 16 - mag_str.size() : mag_str.size() + 1) << ' ' << fx._duration << "s\n" << Color::reset; // << std::setw(10 - std::to_string(fx._magnitude).size()) << " / " << fx._duration << 's' << Color::reset << '\n';
				}
				else os << '\n';
			} };

			if ( cont.size() >= 2 ) { // if at least 2 valid ingredients were found
				if ( cont.size() > 4 ) cont.erase(cont.begin() + 4u, cont.end()); // remove any extra ingredients
				const auto indentation{ std::string(fmt.indent(), ' ') };
				const auto precision{ os.precision() };
				os.precision(fmt.precision());
				os << std::fixed;

				const Potion potion{ cont, gs };

				const auto base_skill{ gs.AlchemyAV() };
				os << Color::f_green << "Potion Builder [Alchemy Skill: " << Color::f_cyan << base_skill;
				if ( const auto mod{ gs.AlchemyMod() }; mod > 0.0 )
					os << Color::reset << "(" << Color::f_green << base_skill + mod << Color::reset << ")";
				os << Color::f_green << ']' << Color::reset << '\n';
				for ( auto& it : cont )
					os << indentation << Color::f_yellow << it._name << '\n';

				if ( const auto name{ potion.name() }; !name.empty() )
					os << name << '\n'; // name

				os << Color::f_red << '{' << Color::reset << '\n'; // Open body

				for ( auto& fx : potion.effects() ) {
					os << indentation << fx._name;
					verbose(fx);
				}

				os << Color::f_red << '}' << Color::reset << '\n'; // Close body

				os.precision(precision); // Reset precision
				return os;
			}
			else throw invalid_param;
		}

		/**
		 * @function print_build_to(std::ostream&, const std::vector<std::string>&, const GameSettings&, const Format& = {})
		 * @brief Construct a potion with the specified ingredients, and output the result to os.
		 * @param os	- Target output stream
		 * @param names	- List of ingredient names ( Min. 2 <= x <= Max. 4 ). If more than 4 ingredients are passed in, the first 4 are used and the rest are discarded.
		 * @param gs	- GameSettings instance, provides the stats necessary for simulating the alchemy algorithm.
		 * @param fmt	- Alchemy Format object, controls how the results appear in the output stream.
		 * @returns std::ostream&
		 */
		std::ostream& print_build_to(std::ostream& os, const std::vector<std::string>& names, const GameSettings& gs, const Format& fmt = Format{ })
		{
			if ( names.size() > 4 ) throw std::exception("Too many ingredients! (Build Mode Max 4)"); else if ( names.size() < 2 ) throw std::exception("Not enough ingredients! (Build Mode Min 2)");
			std::vector<Ingredient> cont;
			for ( auto& it : names ) {
				const auto ingr{ find_ingr(str::tolower(it), !fmt.exact()) };
				if ( ingr != _ingr.end() )
					cont.push_back(*ingr);
				else os << sys::warn << "Couldn't find ingredient: \"" << it << "\"\n";
			}
			if ( cont.size() > 4 ) cont.erase(cont.begin() + 4u, cont.end());
			return print_build_to(os, std::forward<std::vector<Ingredient>>(cont), gs, fmt);
		}

		/**
		 * @function ingredients() const
		 * @brief Retrieve the list of all known ingredients.
		 * @returns IngrList
		 */
		[[nodiscard]] IngrList ingredients() const { return _ingr; }
	};
}
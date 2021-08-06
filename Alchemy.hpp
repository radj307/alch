#pragma once
#include <sstream>
#include <set>
#include <strconv.hpp>
#include "Effect.hpp"
#include "Ingredient.hpp"
//#include "Potion.hpp"

namespace caco_alch {
	namespace _internal {
		/**
		 * @struct less
		 * @brief Custom implementation of the std::less comparison object to handle Ingredient pointers.
		 * @tparam T	- Type to compare
		 */
		template<class T> struct less { bool operator()(const T* a, const T* b) const { return a->_name < b->_name; } };
	}

	class Alchemy {
	public:
		using IngrList = std::vector<Ingredient>;
		using SortedIngrList = std::set<Ingredient*, _internal::less<Ingredient>>;
		using EffectList = std::set<std::string, std::less<std::string>>;
		using InputCont = std::pair<EffectList, IngrList>;
		const std::exception not_found{ "NOT_FOUND" };
		const std::exception invalid_param{ "INVALID_PARAMETERS" };
		short _ingr_color{ Color::_f_white };
	private:
		EffectList _fx;
		IngrList _ingr;

	protected:
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
			for ( auto& fx : _fx ) {
				const auto lower{ str::tolower(fx) };
				if ( str::tolower(fx) == name || ( fuzzy_search && str::tolower(fx).find(name) != std::string::npos ) )
					return true;
			}
			return false;
		}

		/**
		 * @function find_ingr(const std::string&, const size_t = 0)
		 * @brief Retrieve a case-insensitive match from the ingredient list.
		 * @param name	- Name to search for.
		 * @param off	- Position in the list to start search from.
		 * @returns IngrList::iterator
		 */
		IngrList::iterator find_ingr(const std::string& name, const size_t off = 0)
		{
			return std::find_if(_ingr.begin() + off, _ingr.end(), [&name](const IngrList::value_type& ingr) -> bool { return str::tolower(ingr._name) == name; });
		}
		/**
		 * @function get_ingr_list(const std::string&)
		 * @brief Retrieve all ingredients that have a given string in their name.
		 * @param name	- Name to search for.
		 * @returns std::vector<Ingredient>
		 */
		SortedIngrList get_ingr_list(const std::string& name)
		{
			SortedIngrList set;
			for ( auto& it : _ingr )
				if ( str::tolower(it._name) == name || str::tolower(it._name).find(name) != std::string::npos )
					set.insert(&it);
			return std::move(set);
		}
		/**
		 * @function find_ingr_with_effect(const std::string& name)
		 * @brief Retrieve all ingredients that have a given effect.
		 * @param name	- Name to search for.
		 * @returns std::vector<Ingredient>
		 */
		SortedIngrList find_ingr_with_effect(std::string name)
		{
			name = str::tolower(name);
			SortedIngrList set;
			for ( auto& it : _ingr ) {
				for ( auto& fx : it._effects ) {
					const auto lc{ str::tolower(fx._name) };
					if ( lc == name || lc.find(name) != std::string::npos ) {
						set.insert(&it);
						break; // from nested loop
					}
				}
			}
			return std::move(set);
		}

	public:
		/**
		 * @constructor Alchemy(std::pair<EffectList, IngrList>&&)
		 * @brief Default Constructor
		 * @param map	- rvalue ref of loadFromFile output.
		 */
		Alchemy(std::pair<EffectList, IngrList>&& pr) : _fx{ std::move(pr.first) }, _ingr{ std::move(pr.second) } {}

		/**
		 * @function print_search_to(std::ostream&, const std::string&, const bool = false, const bool = false, const size_t = 3)
		 * @brief Retrieve a stringstream containing the information about the given target.
		 * @param os			- Output stream to print to.
		 * @param name			- Name of the target ingredient, or target effect. This is NOT case-sensitive.
		 * @param quietOutput	- When true, does not print the effects of any ingredients that are included in search results.
		 * @param verboseOutput	- When true and quietOutput is false, prints the magnitude of effects.
		 * @param indent		- Target indentation, in space chars.
		 * @returns std::ostream&
		 */
		std::ostream& print_search_to(std::ostream& os, const std::string& name, const bool quietOutput = false, const bool verboseOutput = false, const size_t indent = 3)
		{
			SortedIngrList cont;
			const auto name_lowercase{ str::tolower(name) };
			if ( auto&& tmp{ find_ingr_with_effect(name_lowercase) }; !tmp.empty() )
				cont = std::move(tmp);
			else if ( auto&& tmp{ get_ingr_list(name_lowercase) }; !tmp.empty() )
				cont = std::move(tmp);

			if ( !cont.empty() ) {
				/**
				 * @lambda get_tuple(const std::string&)
				 * @brief Search for name_lowercase in the given string, and split it into a std::tuple of std::string, used for highlighting the search string in the results.
				 * @param str	- Ingredient or effect name to search in.
				 * @returns std::tuple<std::string, std::string, std::string> - First (and only) Element is str if no match was found, or if a match was found, the characters that appear before the match. Second Element contains the matched string to be colorized if a match was found. Third Element contains the characters that appear after a match, if a match was found.
				 */
				const auto get_tuple{ [&name_lowercase](const std::string& str) -> std::tuple<std::string, std::string, std::string> {
					if ( !str.empty() )
						if ( const auto dPos{ str::tolower(str).find(name_lowercase) }; dPos != std::string::npos )
							return { str.substr(0, dPos), str.substr(dPos, name_lowercase.size()), str.substr(dPos + name_lowercase.size()) };
					return{ str, { }, { } };
				} };
				const auto get_quiet_fx{ [&name_lowercase](std::array<Effect, 4>& arr) -> std::vector<Effect*> {
					std::vector<Effect*> vec;
					for ( auto it{ arr.begin() }; it != arr.end(); ++it )
						if ( const auto lc{ str::tolower(it->_name) }; lc == name_lowercase || lc.find(name_lowercase) != std::string::npos )
							vec.push_back(&*it);
					return vec;
				} };
				const auto verbose_output{ [&os](Effect& fx) {
					if ( fx._magnitude > 0.0 )
						os << std::setw(30 - fx._name.size()) << ' ' << std::left << Color::f_gray << fx._magnitude << Color::reset;
				} };

				os << std::fixed; // Set forced standard notation
				const auto precision{ os.precision() }; // copy current output stream precision
				os.precision(2); // Set floating-point-precision to 2 decimal places.

				const auto indentation{ std::string(indent, ' ') }; // get indentation string

				os << Color::f_green << "Search results for: \'" << Color::f_yellow << name << Color::f_green << "\'\n" << Color::f_red << "{\n" << Color::reset;
				size_t i{ 0 };
				for ( auto it{ cont.begin() }; it != cont.end(); ++it, ++i ) {
					const auto name_tuple = get_tuple(( *it )->_name);
					os << indentation;
					sys::colorSet(_ingr_color);
					os << std::get<0>(name_tuple) << Color::reset << Color::f_yellow << std::get<1>(name_tuple) << Color::reset;
					sys::colorSet(_ingr_color);
					os << std::get<2>(name_tuple) << Color::reset << '\n';
					if ( !quietOutput ) // if quietOutput is false
						for ( auto& fx : ( *it )->_effects ) { // iterate through ingredient's effects
							const auto fx_tuple = get_tuple(fx._name);
							os << indentation << indentation << std::get<0>(fx_tuple) << Color::reset << Color::f_yellow << std::get<1>(fx_tuple) << Color::reset << std::get<2>(fx_tuple) << Color::reset;
							if ( verboseOutput ) // check if magnitude should be printed
								verbose_output(fx);
							if ( fx != *( ( *it )->_effects.end() - 1 ) )
								os << '\n';
						}
					else if ( const auto matching_fx{ get_quiet_fx((*it)->_effects) }; !matching_fx.empty() ) // if quietOutput is true
						for ( auto fx{ matching_fx.begin() }; fx != matching_fx.end(); ++fx ) {
							const auto fx_tuple = get_tuple((*fx)->_name);
							os << indentation << indentation << std::get<0>(fx_tuple) << Color::f_yellow << std::get<1>(fx_tuple) << Color::reset << std::get<2>(fx_tuple);
							if ( verboseOutput )
								verbose_output(**fx);
							if ( fx < matching_fx.end() - 1 )
								os << '\n';
						}
					const auto not_last_element{ i < cont.size() - 1 };
					if ( !quietOutput && not_last_element ) // print
						os << '\n';
					if ( not_last_element )
						os << '\n';
				}
				os << Color::f_red << "\n}" << Color::reset << std::endl;
				os.precision(precision); // reset output stream precision
			}
			else // no results found
				os << "Didn't find any ingredients or effects matching \'" << Color::f_yellow << name << Color::reset << "\'\n";
			return os;
		}

		/**
		 * @function print_list_to(std::ostream&, const bool, const bool = false, const bool = false, const size_t = 3)
		 * @brief Insert the ingredient list into an output stream.
		 * @param os				- Target Output Stream.
		 * @param showEffectList	- When true, inserts an additional list of all known effects without ingredients.
		 * @param quietOutput		- When true, doesn't show effects in the ingredient list.
		 * @param verboseOutput		- When true, inserts effect magnitude for each effect.
		 * @param indent			- How many space chars to insert before the ingredient name, and half as many space chars to insert before effect names.
		 * @returns std::ostream&
		 */
		std::ostream& print_list_to(std::ostream& os, const bool showEffectList, const bool quietOutput = false, const bool verboseOutput = false, const size_t indent = 3)
		{
			const auto indentation{ std::string(indent, ' ') };

			os << std::fixed; // force standard notation
			const auto precision{ os.precision() }; // copy the current precision
			os.precision(2); // set precision to 2 decimal places

			const auto ingr_set{ [this]() -> SortedIngrList {
				SortedIngrList set;
				for ( auto it{ _ingr.begin() }; it != _ingr.end(); ++it )
					set.insert(&*it);
				return set;
			}( ) };

			os << Color::f_green << "Ingredients" << Color::f_red << "\n{\n" << Color::reset;
			size_t i{ 0 };
			for ( auto it{ ingr_set.begin() }; it != ingr_set.end(); ++it, ++i ) {
				os << indentation;
				sys::colorSet(_ingr_color); 
				os << ( *it )->_name << Color::reset;
				if ( !quietOutput ) {
					os << '\n';
					for ( auto& fx : ( *it )->_effects ) {
						os << indentation << indentation << fx._name;
						if ( verboseOutput )
							if ( fx._magnitude > 0.0 )
								os << std::setw(30 - fx._name.size()) << ' ' << std::left << Color::f_gray << fx._magnitude << Color::reset;
						if ( fx != *( ( *it )->_effects.end() - 1 ) )
							os << '\n';
					}
					if ( i < ingr_set.size() - 1 )
						os << '\n' << '\n';
				}
			}
			os << Color::f_red << "\n}\n" << Color::reset;
			if ( showEffectList ) {
				os << Color::f_green << "Effects" << Color::f_red << "\n{\n" << Color::reset;
				for ( auto& it : _fx )
					os << indentation << indentation << it << '\n';
				os << Color::f_red << "}\n" << Color::reset;
			}
			os.precision(precision); // reset precision
			return os;
		}

		std::ostream& print_build_to(std::ostream& os, const std::vector<std::string>& names, const bool quietOutput = false, const bool verboseOutput = false, const size_t indent = 3)
		{
			if ( names.size() >= 2 && names.size() <= 4 ) {
				std::vector<Ingredient*> cont;
				for ( auto& it : names ) {
					const auto ingr{ find_ingr(str::tolower(it)) };
					if ( ingr != _ingr.end() )
						cont.push_back(&*ingr);
				}
				if ( cont.size() >= 2 ) { // if at least 2 valid ingredients were found
					const auto indentation{ std::string(indent, ' ') };
					const auto precision{ os.precision() };
					os.precision(2);

					// TODO: Implement potion building

					os.precision(precision);
					return os;
				}
			}
			throw invalid_param;
		}
	};
}
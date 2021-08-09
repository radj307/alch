#pragma once
#include <sstream>
#include <set>
#include <strconv.hpp>
#include "Common.hpp"
#include "Effect.hpp"
#include "Ingredient.hpp"
#include "Potion.hpp"

namespace caco_alch {
	class Alchemy {
	public:
		using IngrList = std::vector<Ingredient>;
		using SortedIngrList = std::set<Ingredient*, _internal::less<Ingredient>>;
		const std::exception not_found{ "NOT_FOUND" };
		const std::exception invalid_param{ "INVALID_PARAMETERS" };
	private:
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
				for ( auto& fx : it._effects ) {
					if ( str::tolower(fx._name) == name || (fuzzy_search && str::tolower(fx._name).find(name) != std::string::npos) )
						return true;
				}
			}
			return false;
		}

		IngrList& get_ingr_ref() { return _ingr; }
		std::set<Effect> getEffectList()
		{
			std::set<Effect> ret{ };
			for ( auto& i : _ingr )
				for ( auto& fx : i._effects )
					ret.insert({ fx._name, static_cast<double>(-0.0) });
			return ret;
			/*
			std::vector<Effect> vec;
			const auto push{ [&vec](const Effect& fx) {
				bool is_dupl{ false };
				for ( const auto& it : vec )
					if ( it == fx )
						is_dupl = true;
				if ( !is_dupl )
					vec.push_back(fx);
			} };
			for ( auto& it : _ingr )
				for ( auto& fx : it._effects )
					push(fx);
			return vec;*/
		}

		/**
		 * @function find_ingr(const std::string&, const size_t = 0)
		 * @brief Retrieve a case-insensitive match from the ingredient list.
		 * @param name	- Name to search for.
		 * @param off	- Position in the list to start search from.
		 * @returns IngrList::iterator
		 */
		IngrList::iterator find_ingr(const std::string& name, const bool fuzzy_search, const size_t off = 0)
		{
			return std::find_if(_ingr.begin() + off, _ingr.end(), [&name](const IngrList::value_type& ingr) -> bool { return str::tolower(ingr._name) == name; });
		}
		/**
		 * @function get_ingr_list(const std::string&)
		 * @brief Retrieve all ingredients that have a given string in their name.
		 * @param name	- Name to search for.
		 * @returns std::vector<Ingredient>
		 */
		SortedIngrList get_ingr_list(const std::string& name, const bool fuzzy_search)
		{
			SortedIngrList set;
			for ( auto& it : _ingr )
				if ( str::tolower(it._name) == name || ( fuzzy_search && str::tolower(it._name).find(name) != std::string::npos ) )
					set.insert(&it);
			return std::move(set);
		}
		/**
		 * @function find_ingr_with_effect(const std::string& name)
		 * @brief Retrieve all ingredients that have a given effect.
		 * @param name	- Name to search for.
		 * @returns std::vector<Ingredient>
		 */
		SortedIngrList find_ingr_with_effect(std::string name, const bool fuzzy_search)
		{
			name = str::tolower(name);
			SortedIngrList set;
			for ( auto& it : _ingr ) {
				for ( auto& fx : it._effects ) {
					const auto lc{ str::tolower(fx._name) };
					if ( lc == name || ( fuzzy_search && lc.find(name) != std::string::npos ) ) {
						set.insert(&it);
						break; // from nested loop
					}
				}
			}
			return std::move(set);
		}

		/**
		 * @constructor Alchemy(std::pair<EffectList, IngrList>&&)
		 * @brief Default Constructor
		 * @param map	- rvalue ref of loadFromFile output.
		 */
		Alchemy(IngrList&& ingr) : _ingr{ std::move(ingr) } {}

		/**
		 * @struct Format
		 * @brief Provides formatting information for some output stream methods in the Alchemy class.
		 */
		struct Format {
		private:
			bool _quiet, _verbose, _exact, _all; // see UserAssist.hpp
			size_t _indent, _precision;
			unsigned short _color;

		public:
			Format(const bool quiet = false, const bool verbose = true, const bool exact = false, const bool all = false, const size_t indent = 3, const size_t precision = 2, const unsigned short color = Color::_f_white) : _quiet{ quiet }, _verbose{ verbose }, _exact{ exact }, _all{ all }, _indent{ indent }, _precision{ precision }, _color{ color } {}

			bool quiet() const			{ return _quiet; }
			bool verbose() const		{ return _verbose; }
			bool exact() const			{ return _exact; }
			bool all() const			{ return _all; }
			size_t indent() const		{ return _indent; }
			size_t precision() const	{ return _precision; }
			short color() const			{ return _color; }

			Format& operator()() { return *this; }
		};

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
		std::ostream& print_search_to(std::ostream& os, const std::string& name, const Format& fmt = { })
		{
			const auto indentation{ std::string(fmt.indent(), ' ') }; // get indentation string
			SortedIngrList cont;
			const auto name_lowercase{ str::tolower(name) };
			if ( auto&& tmp{ find_ingr_with_effect(name_lowercase, !fmt.exact()) }; !tmp.empty() )
				cont = std::move(tmp);
			else if ( auto&& tmp{ get_ingr_list(name_lowercase, !fmt.exact()) }; !tmp.empty() )
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
				const auto get_quiet_fx{ [&name_lowercase](std::array<Effect, 4>& arr, const bool exact_only) -> std::vector<Effect*> {
					std::vector<Effect*> vec;
					for ( auto it{ arr.begin() }; it != arr.end(); ++it )
						if ( const auto lc{ str::tolower(it->_name) }; lc == name_lowercase || ( !exact_only && lc.find(name_lowercase) != std::string::npos ) ) {
							vec.push_back(&*it);
							if ( exact_only ) break;
						}
					return vec;
				} };
				const auto verbose{ [&os, &fmt](Effect& fx) {
					if ( ( fx._magnitude > 0.0 && fmt.verbose() ) || fmt.all() ) {
						if ( fx._name.size() > 30u ) // name is long
							os << std::setw(fx._name.size()) << ' ';
						else // name is normal length
							os << std::setw(30 - fx._name.size()) << ' ';
						os << Color::f_gray << fx._magnitude << Color::reset << '\n';
					}
					else os << '\n';
				} };

				os << std::fixed; // Set forced standard notation
				const auto precision{ os.precision() }; // copy current output stream precision
				os.precision(fmt.precision()); // Set floating-point-precision to 2 decimal places.

				os << Color::f_green << "Search results for: \'" << Color::f_yellow << name << Color::f_green << "\'\n" << Color::f_red << "{\n" << Color::reset;
				size_t i{ 0 };
				for ( auto it{ cont.begin() }; it != cont.end(); ++it, ++i ) {
					const auto name_tuple = get_tuple(( *it )->_name);
					os << indentation;
					sys::colorSet(fmt.color());
					os << std::get<0>(name_tuple) << Color::reset << Color::f_yellow << std::get<1>(name_tuple) << Color::reset;
					sys::colorSet(fmt.color());
					os << std::get<2>(name_tuple) << Color::reset << '\n';
					if ( !fmt.quiet() || ( fmt.verbose() || fmt.all() ) ) // if quietOutput is false
						for ( auto& fx : ( *it )->_effects ) { // iterate through ingredient's effects
							const auto fx_tuple = get_tuple(fx._name);
							os << indentation << indentation << std::get<0>(fx_tuple) << Color::reset << Color::f_yellow << std::get<1>(fx_tuple) << Color::reset << std::get<2>(fx_tuple);
							verbose(fx);
						}
					else if ( const auto matching_fx{ get_quiet_fx(( *it )->_effects, fmt.exact()) }; !matching_fx.empty() ) // if quietOutput is true
						for ( auto fx{ matching_fx.begin() }; fx != matching_fx.end(); ++fx ) {
							const auto fx_tuple = get_tuple(( *fx )->_name);
							os << indentation << indentation << std::get<0>(fx_tuple) << Color::f_yellow << std::get<1>(fx_tuple) << Color::reset << std::get<2>(fx_tuple);
							verbose(**fx);
						}
					const auto not_last_element{ i < cont.size() - 1 };
				}
				os << Color::f_red << "}" << Color::reset << std::endl;
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
		 * @param showEffectList	- When true, inserts an additional list of all known effects without ingredients.
		 * @param quietOutput		- When true, doesn't show effects in the ingredient list.
		 * @param verboseOutput		- When true, inserts effect magnitude for each effect.
		 * @param indent			- How many space chars to insert before the ingredient name, and half as many space chars to insert before effect names.
		 * @returns std::ostream&
		 */
		std::ostream& print_list_to(std::ostream& os, const Format& fmt = { })
		{
			const auto indentation{ std::string(fmt.indent(), ' ') };

			os << std::fixed; // force standard notation
			const auto precision{ os.precision() }; // copy the current precision
			os.precision(fmt.precision()); // set precision to 2 decimal places

			const auto ingr_set{ [this]() -> SortedIngrList {
				SortedIngrList set;
				for ( auto it{ _ingr.begin() }; it != _ingr.end(); ++it )
					set.insert(&*it);
				return set;
			}( ) };

			const auto verbose{ [&os, &fmt](Effect& fx) {
				if ( ( fx._magnitude > 0.0 && fmt.verbose() ) || fmt.all() ) {
					if ( fx._name.size() > 30u ) // name is long
						os << std::setw(fx._name.size()) << ' ';
					else // name is normal length
						os << std::setw(30 - fx._name.size()) << ' ';
					os << Color::f_gray << fx._magnitude << Color::reset << '\n';
				}
				else os << '\n';
			} };

			os << Color::f_green << "Ingredients" << Color::f_red << "\n{\n" << Color::reset;
			size_t i{ 0 };
			for ( auto it{ ingr_set.begin() }; it != ingr_set.end(); ++it, ++i ) {
				os << indentation;
				sys::colorSet(fmt.color());
				os << ( *it )->_name << Color::reset << '\n';
				if ( !fmt.quiet() || fmt.all() ) {
					for ( auto& fx : ( *it )->_effects ) {
						os << indentation << indentation << fx._name;
						verbose(fx);
					}
				}
			}
			os << Color::f_red << "}\n" << Color::reset;
			if ( fmt.all() ) {
				const auto fx{ getEffectList() };
				os << Color::f_green << "Effects" << Color::f_red << "\n{\n" << Color::reset;
				for ( auto& it : fx )
					os << indentation << indentation << it._name << '\n';
				os << Color::f_red << "}\n" << Color::reset;
			}
			os.precision(precision); // reset precision
			return os;
		}

		/**
		 * @function print_build_to(std::ostream&, const std::vector<std::string>&, const GameSettings&, const Format& = {})
		 * @brief Construct a potion with the specified ingredients, and output the result to os.
		 * @param os	- Target output stream
		 * @param names	- List of ingredient names ( Min. 2 <= x <= Max. 4 ).
		 * @param gs	- GameSettings instance, provides the stats necessary for simulating the alchemy algorithm.
		 * @param fmt	- Alchemy Format object, controls how the results appear in the output stream.
		 * @returns std::ostream&
		 */
		std::ostream& print_build_to(std::ostream& os, const std::vector<std::string>& names, const GameSettings& gs, const Format& fmt = { })
		{
			if ( names.size() > 4 ) throw std::exception("Too many ingredients! (Build Mode Max 4)"); else if ( names.size() < 2 ) throw std::exception("Not enough ingredients! (Build Mode Min 2)");
			std::vector<Ingredient> cont;
			for ( auto& it : names ) {
				const auto ingr{ find_ingr(str::tolower(it), !fmt.exact()) };
				if ( ingr != _ingr.end() )
					cont.push_back(*ingr);
				else os << sys::warn << "Couldn't find ingredient: \"" << it << "\"\n";
			}

			const auto verbose{ [&os, &fmt](Effect& fx, const size_t duration) {
				if ( ( fx._magnitude != 0.0 && ( fmt.verbose() || !fmt.quiet() ) ) || fmt.all() ) {
					if ( fx._name.size() > 30u ) // name is long
						os << std::setw(fx._name.size()) << ' ';
					else // name is normal length
						os << std::setw(30 - fx._name.size()) << ' ';
					os << Color::f_gray << fx._magnitude << std::setw(10 - std::to_string(fx._magnitude).size()) << " / " << duration << 's' << Color::reset << '\n';
				}
				else os << '\n';
			} };

			if ( cont.size() >= 2 ) { // if at least 2 valid ingredients were found
				const auto indentation{ std::string(fmt.indent(), ' ') };
				const auto precision{ os.precision() };
				os.precision(fmt.precision());
				os << std::fixed;

				Potion potion{ std::move(cont), gs };

				os << Color::f_green << "Build Mode Result [LVL: " << Color::f_cyan << gs._alchemy_skill << Color::f_green << ']' << Color::reset << '\n';
				for ( auto& it : cont )
					os << indentation << Color::f_yellow << it._name << '\n';

				const auto name{ potion.name() };
				if ( !name.empty() )
					os << name << '\n'; // name

				os << Color::f_red << '{' << Color::reset << '\n'; // Open body

				for ( auto& fx : potion.effects() ) {
					os << indentation << fx._name;
					verbose(fx, potion.duration());
				}

				os << Color::f_red << '}' << Color::reset << '\n'; // Close body

				os.precision(precision); // Reset precision
				return os;
			}
			else throw invalid_param;
		}

		IngrList ingredients() const { return _ingr; }
	};
}
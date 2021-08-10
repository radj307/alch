#pragma once
#include <sstream>
#include <set>
#include <strconv.hpp>
#include "Effect.hpp"
#include "Ingredient.hpp"
#include "Potion.hpp"

/**
 * @namespace caco_alch
 * @brief Contains everything used by the caco-alch project.
 */
namespace caco_alch {
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
		template<class T>
		struct less {
			bool operator()(const T* a, const T* b) const { return a->_name < b->_name; }
		};
	}
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
					ret.insert(Effect{ fx._name, static_cast<double>(-0.0), static_cast<unsigned>(0) });
			return ret;
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
			bool _quiet, _verbose, _exact, _all, _file_export, _reverse_output; // see UserAssist.hpp
			size_t _indent, _precision;
			unsigned short _color;

		public:
			/**
			 * @constructor Format(const bool = false, const bool = true, const bool = false, const bool = false, const size_t = 3u, const size_t = 2u, const unsigned short = Color::_f_white)
			 * @brief Default Constructor
			 * @param quiet		- When true, only includes effects that match part of the search string in results.
			 * @param verbose	- When true, includes additional information about an effect's magnitude and duration.
			 * @param exact		- When true, only includes exact matches in results.
			 * @param all		- When true, includes all additional information in results.
			 * @param indent	- How many space characters to include before ingredient names. This is multiplied by 2 for effect names.
			 * @param precision	- How many decimal points of precision to use when outputting floating points.
			 * @param color		- General color override, changes the color of Ingredient names for search, list, and build.
			 */
			Format(const bool quiet = false, const bool verbose = true, const bool exact = false, const bool all = false, const bool file_export = false, const bool reverse_output = false, const size_t indent = 3, const size_t precision = 2u, const unsigned short color = Color::_f_white) : _quiet{ quiet }, _verbose{ verbose }, _exact{ exact }, _all{ all }, _file_export{ file_export }, _reverse_output{ reverse_output }, _indent{ indent }, _precision{ precision }, _color{ color } {}

			bool quiet() const { return _quiet; }
			bool verbose() const { return _verbose; }
			bool exact() const { return _exact; }
			bool all() const { return _all; }
			bool file_export() const { return _file_export; }
			bool reverse_output() const { return _reverse_output; }
			size_t indent() const { return _indent; }
			size_t precision() const { return _precision; }
			short color() const { return _color; }

			std::tuple<std::string, std::string, std::string> get_tuple(const std::string& str, const std::string& name_lowercase) const
			{
				if ( !str.empty() )
					if ( const auto dPos{ str::tolower(str).find(name_lowercase) }; dPos != std::string::npos )
						return { str.substr(0, dPos), str.substr(dPos, name_lowercase.size()), str.substr(dPos + name_lowercase.size()) };
				return{ str, { }, { } };
			}

			std::vector<Effect*> get_fx(std::array<Effect, 4>& arr, const std::string& name_lowercase) const
			{
				std::vector<Effect*> vec;
				for ( auto it{ arr.begin() }; it != arr.end(); ++it ) {
					if ( !_quiet )
						vec.push_back(&*it);
					else if ( const auto lc{ str::tolower(it->_name) }; lc == name_lowercase || ( !_exact && lc.find(name_lowercase) != std::string::npos ) ) {
						vec.push_back(&*it);
						if ( _exact ) break;
					}
				}
				return vec;
			}

			/**
			 * @function to_fstream(std::ostream&, const Ingredient&) const
			 * @brief Insert a registry-formatted ingredient into an output stream.
			 * @param os	- Target output stream.
			 * @param ingr	- Target ingredient.
			 * @returns std::ostream&
			 */
			std::ostream& to_fstream(std::ostream& os, const Ingredient& ingr) const
			{
				os << ingr._name << "\n{\n";
				for ( auto& fx : ingr._effects )
					os << '\t' << fx._name << "\n\t{\n\t\tmagnitude = " << fx._magnitude << "\n\t\tduration = " << fx._duration << "\n\t}\n";
				os << "}\n";
				return os;
			}

			/**
			 * @function to_fstream(std::ostream&, const SortedIngrList&) const
			 * @brief Insert a registry-formatted list of ingredients into an output stream.
			 * @param os	- Target output stream.
			 * @param ingr	- Target ingredient list.
			 * @returns std::ostream&
			 */
			std::ostream& to_fstream(std::ostream& os, const SortedIngrList& ingr) const
			{
				if ( _reverse_output )
					for ( auto it{ ingr.rbegin() }; it != ingr.rend(); ++it )
						to_fstream(os, **it);
				else
					for ( auto it{ ingr.begin() }; it != ingr.end(); ++it )
						to_fstream(os, **it);
				return os;
			}

			std::ostream& to_stream(std::ostream& os, const Effect& fx, const std::string& search_str, const std::string& indentation) const
			{
				const auto fx_tuple{ get_tuple(fx._name, search_str) };
				os << indentation << indentation << std::get<0>(fx_tuple) << Color::reset << Color::f_yellow << std::get<1>(fx_tuple) << Color::reset << std::get<2>(fx_tuple);
				if ( fx._magnitude > 0.0 || _all ) {
					if ( fx._name.size() > 30u ) // name is long
						os << std::setw(fx._name.size()) << ' ';
					else // name is normal length
						os << std::setw(30 - fx._name.size()) << ' ';
					os << Color::f_gray << fx._magnitude;
					if ( _verbose || _all )
						os << '\t' << fx._duration << 's';
					os << Color::reset << '\n';
				}
				else os << '\n';
				return os;
			}
			std::ostream& to_stream(std::ostream& os, Effect* fx, const std::string& search_str, const std::string& indentation) const
			{
				const auto fx_tuple{ get_tuple(fx->_name, search_str) };
				os << indentation << indentation << std::get<0>(fx_tuple) << Color::reset << Color::f_yellow << std::get<1>(fx_tuple) << Color::reset << std::get<2>(fx_tuple);
				if ( fx->_magnitude > 0.0 || _all ) {
					if ( fx->_name.size() > 30u ) // name is long
						os << std::setw(fx->_name.size()) << ' ';
					else // name is normal length
						os << std::setw(30 - fx->_name.size()) << ' ';
					os << Color::f_gray << fx->_magnitude;
					if ( _verbose || _all )
						os << '\t' << fx->_duration << 's';
					os << Color::reset << '\n';
				}
				else os << '\n';
				return os;
			}
			std::ostream& to_stream(std::ostream& os, Ingredient& ingr, const std::string& search_str) const
			{
				const auto indentation{ std::string(_indent, ' ') }; // get indentation string
				const auto name_tuple{ get_tuple(ingr._name, search_str) };
				os << indentation;
				sys::colorSet(_color);
				os << std::get<0>(name_tuple) << Color::reset << Color::f_yellow << std::get<1>(name_tuple) << Color::reset;
				sys::colorSet(_color);
				os << std::get<2>(name_tuple) << Color::reset << '\n';
				const auto effects{ get_fx(ingr._effects, search_str) };
				for ( auto& fx : effects )
					to_stream(os, fx, search_str, indentation);
				return os;
			}
			std::ostream& to_stream(std::ostream& os, const Effect& fx, const std::string& indentation) const
			{
				os << indentation << indentation << fx._name << std::setw(30 - fx._name.size()) << ' ' << Color::f_gray;
				if ( fx._magnitude > 0.0 || _all )
					os << fx._magnitude;
				else os << '\t';
				if ( fx._duration > 0 || _all )
					os << '\t' << fx._duration << 's';
				os << Color::reset << '\n';
				return os;
			}
			std::ostream& to_stream(std::ostream& os, Ingredient& ingr) const
			{
				const auto indentation{ std::string(_indent, ' ') }; // get indentation
				os << indentation;
				if ( _color != Color::_f_white )
					sys::colorSet(_color);
				else sys::colorSet(Color::_f_yellow);
				os << ingr._name << Color::reset << '\n';
				if ( !_quiet && !_all )
					for ( auto& fx : ingr._effects )
						to_stream(os, fx, indentation);
				return os;
			}

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
			SortedIngrList cont;
			const auto name_lowercase{ str::tolower(name) };
			if ( auto&& tmp{ find_ingr_with_effect(name_lowercase, !fmt.exact()) }; !tmp.empty() )
				cont = std::move(tmp);
			else if ( auto&& tmp{ get_ingr_list(name_lowercase, !fmt.exact()) }; !tmp.empty() )
				cont = std::move(tmp);

			if ( !cont.empty() ) {
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
					/*	size_t i{ 0 };
						for ( auto it{ cont.begin() }; it != cont.end(); ++it, ++i ) {
							const auto name_tuple = get_tuple(( *it )->_name);
							os << indentation;
							sys::colorSet(fmt.color());
							os << std::get<0>(name_tuple) << Color::reset << Color::f_yellow << std::get<1>(name_tuple) << Color::reset;
							sys::colorSet(fmt.color());
							os << std::get<2>(name_tuple) << Color::reset << '\n';
							if ( !fmt.quiet() || fmt.all() ) // if quietOutput is false
								for ( auto& fx : ( *it )->_effects ) { // iterate through ingredient's effects
									const auto fx_tuple = get_tuple(fx._name);
									os << indentation << indentation << std::get<0>(fx_tuple) << Color::reset << Color::f_yellow << std::get<1>(fx_tuple) << Color::reset << std::get<2>(fx_tuple);
									print_fx_stats(fx);
								}
							else if ( const auto matching_fx{ get_quiet_fx(( *it )->_effects, fmt.exact()) }; !matching_fx.empty() ) // if quietOutput is true
								for ( auto fx{ matching_fx.begin() }; fx != matching_fx.end(); ++fx ) {
									const auto fx_tuple = get_tuple(( *fx )->_name);
									os << indentation << indentation << std::get<0>(fx_tuple) << Color::f_yellow << std::get<1>(fx_tuple) << Color::reset << std::get<2>(fx_tuple);
									print_fx_stats(**fx);
								}
							const auto not_last_element{ i < cont.size() - 1 };
						}*/
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
		 * @param showEffectList	- When true, inserts an additional list of all known effects without ingredients.
		 * @param quietOutput		- When true, doesn't show effects in the ingredient list.
		 * @param verboseOutput		- When true, inserts effect magnitude for each effect.
		 * @param indent			- How many space chars to insert before the ingredient name, and half as many space chars to insert before effect names.
		 * @returns std::ostream&
		 */
		std::ostream& print_list_to(std::ostream& os, const Format& fmt = { })
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
		std::ostream& print_build_to(std::ostream& os, IngrList&& cont, const GameSettings& gs, const Format& fmt = { })
		{
			const auto verbose{ [&os, &fmt](Effect& fx) {
				if ( ( fx._magnitude != 0.0 && ( fmt.verbose() || !fmt.quiet() ) ) || fmt.all() ) {
					if ( fx._name.size() > 30u ) // name is long
						os << std::setw(fx._name.size()) << ' ';
					else // name is normal length
						os << std::setw(30 - fx._name.size()) << ' ';
					os << Color::f_gray << fx._magnitude << std::setw(10 - std::to_string(fx._magnitude).size()) << " / " << fx._duration << 's' << Color::reset << '\n';
				}
				else os << '\n';
			} };

			if ( cont.size() >= 2 ) { // if at least 2 valid ingredients were found
				if ( cont.size() > 4 ) cont.erase(cont.begin() + 4u, cont.end()); // remove any extra ingredients
				const auto indentation{ std::string(fmt.indent(), ' ') };
				const auto precision{ os.precision() };
				os.precision(fmt.precision());
				os << std::fixed;

				Potion potion{ std::forward<IngrList>(cont), gs };

				os << Color::f_green << "Potion Builder [Alchemy Skill: " << Color::f_cyan << gs._alchemy_skill << Color::f_green << ']' << Color::reset << '\n';
				for ( auto& it : cont )
					os << indentation << Color::f_yellow << it._name << '\n';

				const auto name{ potion.name() };
				if ( !name.empty() )
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
			if ( cont.size() > 4 ) cont.erase(cont.begin() + 4u, cont.end());
			return print_build_to(os, std::move(cont), gs, fmt);
		}

		/**
		 * @function ingredients() const
		 * @brief Retrieve the list of all known ingredients.
		 * @returns IngrList
		 */
		IngrList ingredients() const { return _ingr; }
	};
}
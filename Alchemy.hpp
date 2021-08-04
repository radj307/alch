#pragma once
#include <sstream>
#include <strconv.hpp>
#include "Ingredient.hpp"

class Alchemy {
public:
	using IngrList = std::vector<Ingredient>;
	using EffectList = std::map<std::string, IngrList>;
	using InputCont = std::pair<EffectList, IngrList>;
	using Except = std::exception;
private:
	const Except not_found{ "NOT_FOUND" };
	EffectList _fx_map;
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
	bool is_ingr(const std::string& name) { return std::any_of(_ingr.begin(), _ingr.end(), [&name](Ingredient& ingr) { return str::tolower(ingr._name) == name; }); }
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
		for ( auto& [fx, ingr] : _fx_map ) {
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
		return std::find_if(_ingr.begin() + off, _ingr.end(), [&name](Ingredient& ingr) -> bool { return str::tolower(ingr._name) == name; });
	}
	/**
	 * @function get_ingr_list(const std::string&)
	 * @brief Retrieve all ingredients that have a given string in their name.
	 * @param name	- Name to search for.
	 * @returns std::vector<Ingredient>
	 */
	std::vector<Ingredient*> get_ingr_list(const std::string& name)
	{
		std::vector<Ingredient*> vec;
		for ( auto& it : _ingr )
			if ( str::tolower(it._name) == name || str::tolower(it._name).find(name) != std::string::npos )
				vec.push_back(&it);
		return vec;
	}
	/**
	 * @function find_ingr_with_effect(const std::string& name)
	 * @brief Retrieve all ingredients that have a given effect.
	 * @param name	- Name to search for.
	 * @returns std::vector<Ingredient>
	 */
	std::vector<Ingredient>* find_ingr_with_effect(const std::string& name) 
	{
		for ( auto& [fx, ingr_vec] : _fx_map )
			if ( str::tolower(fx) == name || str::tolower(fx).find(name) != std::string::npos )
				return &ingr_vec;
		return { };
	}

public:
	/**
	 * @constructor Alchemy(std::pair<EffectList, IngrList>&&)
	 * @brief Default Constructor
	 * @param map	- rvalue ref of loadFromFile output.
	 */
	Alchemy(std::pair<EffectList, IngrList>&& map) : _fx_map{ std::move(map.first) }, _ingr { std::move(map.second) } {}

	/**
	 * @function get_ingr_vec()
	 * @brief Retrieve a reference to the ingredient list.
	 * @returns IngrList&
	 */
	IngrList& get_ingr_vec() { return _ingr; }

	/**
	 * @function get_effect_map()
	 * @brief Retrieve a reference to the effect list.
	 * @returns EffectList&
	 */
	EffectList& get_effect_map() { return _fx_map; }

	/**
	 * @function print_search_to(std::ostream&, const std::string&, const bool = false, const bool = false, const size_t = 3)
	 * @brief Retrieve a stringstream containing the information about the given target.
	 * @param os			- Output stream to print to.
	 * @param name			- Name of the target ingredient, or target effect. This is NOT case-sensitive.
	 * @param quietOutput	- When true, does not print the effects of any ingredients that are included in search results.
	 * @param verboseOutput	- When true and quietOutput is false, prints the magnitude of effects.
	 * @param indent		- Target indentation, in space chars.
	 */
	void print_search_to(std::ostream& os, const std::string& name, const bool quietOutput = false, const bool verboseOutput = false, const size_t indent = 3)
	{
		std::vector<Ingredient*> cont;
		const auto name_lowercase{ str::tolower(name) };
		if ( auto&& tmp{ find_ingr_with_effect(name_lowercase) }; !tmp->empty() ) {
			cont.reserve(tmp->size());
			for ( auto& it : *tmp )
				cont.push_back(&it);
			cont.shrink_to_fit();
		}
		else if ( auto&& tmp{ get_ingr_list(name_lowercase) }; !tmp.empty() )
			cont = tmp;

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

			os << std::fixed; // Set forced standard notation
			const auto precision{ os.precision() }; // copy current output stream precision
			if ( verboseOutput )
				os.precision(2); // Set floating-point-precision to 2 decimal places.

			const auto indentation{ std::string(indent, ' ') }; // get indentation string

			os << Color::f_green << "Search results for: \'" << Color::f_yellow << name << Color::f_green << "\'\n" << Color::f_red << "{\n" << Color::reset;
			for ( auto it{ cont.begin() }; it != cont.end(); ++it ) {
				const auto name_tuple = get_tuple(( *it )->_name);
				os << indentation << std::get<0>(name_tuple) << Color::f_yellow << std::get<1>(name_tuple) << Color::reset << std::get<2>(name_tuple);
				if ( !quietOutput ) {
					os << '\n';
					for ( auto& fx : ( *it )->_effects ) { // iterate through ingredient's effects
						const auto fx_tuple = get_tuple(fx._name);
						os << indentation << indentation << std::get<0>(fx_tuple) << Color::f_yellow << std::get<1>(fx_tuple) << Color::reset << std::get<2>(fx_tuple);
						if ( verboseOutput ) // check if magnitude should be printed
							if ( fx._magnitude > 0.0 )
								os << std::setw(30 - fx._name.size()) << ' ' << std::left << Color::f_gray << fx._magnitude << Color::reset;
						os << '\n';
					}
				}
				if ( it != cont.end() )
					os << '\n';
			}
			os << Color::f_red << "}" << Color::reset << std::endl;
			os.precision(precision); // reset output stream precision
		}
		else { // if no results were found, return early.
			os << "Didn't find any ingredients or effects matching \'" << Color::f_yellow << name << Color::reset << "\'\n";
			return;
		}
	}

	/**
	 * @function list()
	 * @brief Returns a stringstream with a formatted list of all ingredients & their effects.
	 * @returns std::stringstream
	 */
	std::stringstream list()
	{
		std::stringstream ss;
		for ( auto& it : _ingr )
			ss << it << '\n';
		return std::move(ss);
	}

	/**
	 * @function operator<<(std::ostream&, const Alchemy&)
	 * @brief Stream insertion operator, returns a formatted list of all ingredients & their effects, as well as a list of all known effects.
	 * @param os	- (implicit) Output stream instance.
	 * @param alch	- (implicit) Alchemy instance.
	 * @returns std::ostream&
	 */
	friend std::ostream& operator<<(std::ostream& os, Alchemy& alch)
	{
		os << "All Ingredients\n{\n";
		// insert ingredients list
		for ( auto& ingr : alch._ingr ) {
			ingr._indent = 1;
			os << ingr << '\n';
		}
		os << "}\nAll Effects\n{\n";
		// insert effects list
		for ( auto& [fx, ingredients] : alch._fx_map )
			os << '\t' << fx << '\n';
		os << "}" << std::endl;
		return os;
	}
};
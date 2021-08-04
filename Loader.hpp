#pragma once
#include <algorithm>
#include <file.h>
#include "Alchemy.hpp"

/**
 * @function strip_line(std::string, const Param&)
 * @brief Removes comments and preceeding/trailing whitespace from a given string.
 * @param str	- Input string.
 * @param cfg	- Configuration.
 * @returns std::string
 */
inline std::string strip_line(std::string str)
{
	if ( const auto dPos{ str.find_first_of("#;") }; !str.empty() && dPos != std::string::npos ) // remove comments first
		str.erase(str.begin() + dPos, str.end());
	if ( const auto first{ str.find_first_not_of(" \t\r\n\v") }; !str.empty() && first != std::string::npos )
		str.erase(std::remove_if(str.begin(), str.begin() + first, isspace), str.begin() + first); // remove from beginning of string to 1 before first non-whitespace char
	else return{ };
	if ( const auto last{ str.find_last_not_of(" \t\r\n\v") }; !str.empty() && last != std::string::npos )
		str.erase(std::remove_if(str.begin() + last, str.end(), isspace), str.end()); // remove from 1 after last non-whitespace char to the end of the string
	return str;
}

/**
 * @function loadFromFile(const std::string&)
 * @brief Load an alchemy effect list & ingredient list from a file.
 * @param filename	- Name of target file.
 * @returns std::pair<Alchemy::EffectList, Alchemy::IngrList>
 */
inline std::pair<Alchemy::EffectList, Alchemy::IngrList> loadFromFile(const std::string& filename)
{
	Alchemy::EffectList map;
	Alchemy::IngrList vec;
	auto ss{ file::read(filename) };

	if ( ss.fail() ) throw std::exception("Couldn't find ingredient registry! (Is it located in the same directory?)");

	std::string name;
	std::array<Effect, 4> effects;
	size_t insert_at{ 0 };

	bool insert_into = false;

	for ( std::string ln{ }; std::getline(ss, ln); ) {
		ln = strip_line(ln);
		const auto is_open{ ln.find('{') != std::string::npos }, is_close{ ln.find('}') != std::string::npos };
		if ( is_open )
			insert_into = true;
		else if ( is_close ) {
			vec.emplace_back(Ingredient(name, effects));
			for ( auto& it : effects )
				map[it._name].push_back(Ingredient(name, effects));
			name = "";
			effects = { };
			insert_at = 0;
			insert_into = false;
		}
		else if ( insert_into ) {
			const auto eqPos{ ln.find('=') };
			if ( eqPos != std::string::npos ) { // add effect with magnitude
				const auto name{ strip_line(ln.substr(0, eqPos)) }, mag{ strip_line(ln.substr(eqPos + 1)) };
				const auto num{ std::stod(mag) };
				effects.at(insert_at++) = { name, num };
			}
			else // add effect without magnitude
				effects.at(insert_at++) = { ln, -0.0 };
		}
		else name = ln;
	}

	return { map, vec };
}
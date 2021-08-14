#pragma once
#include <algorithm>
#include <file.h>
#include <sysapi.h>
#include <xINI.hpp>
#include "Alchemy.hpp"

namespace caco_alch {
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
	 * @function parseFileContent(const std::string&)
	 * @brief Parse a stringstream containing valid ingredient registry data.
	 * @param ss	- Stringstream rvalue.
	 * @returns Alchemy::IngrList
	 */
	inline Alchemy::IngrList parseFileContent(std::stringstream&& ss)
	{
		if ( ss.fail() ) throw std::exception("Cannot parse stringstream -- Failbit is set!");

		Alchemy::IngrList ingredients;

		std::string tmp_name;
		std::array<Effect, 4> tmp_fx;
		size_t insert_at{ 0 };

		bool insert_into = false;

		for ( std::string ln{ }; std::getline(ss, ln); ) {
			ln = strip_line(ln);
			const auto is_open{ ln.find('{') != std::string::npos }, is_close{ ln.find('}') != std::string::npos };
			if ( is_open )
				insert_into = true;
			else if ( is_close ) {
				if ( std::find_if(ingredients.begin(), ingredients.end(), [&tmp_name](Alchemy::IngrList::value_type ingr) { return ingr._name == tmp_name; }) == ingredients.end() ) {
					ingredients.emplace_back(Ingredient(tmp_name, tmp_fx));
				}
				tmp_name = "";
				tmp_fx = { };
				insert_at = 0;
				insert_into = false;
			}
			else if ( insert_into ) {
				const auto eqPos{ ln.find('=') };
				if ( eqPos != std::string::npos ) { // add effect with magnitude
					const auto tmp_name{ strip_line(ln.substr(0, eqPos)) }, mag{ strip_line(ln.substr(eqPos + 1)) };
					const auto num{ std::stod(mag) };
					tmp_fx.at(insert_at++) = { tmp_name, num };
				}
				else // add effect without magnitude
					tmp_fx.at(insert_at++) = { ln, -0.0 };
			}
			else tmp_name = ln;
		}
		return ingredients;
	}

	/**
	 * @function loadFromFile(const std::string&)
	 * @brief Load an ingredient list from a file.
	 * @param filename	- Name of target file.
	 * @returns Alchemy::IngrList
	 */
	inline Alchemy::IngrList loadFromFile(const std::string& filename)
	{
		auto ss{ file::read(filename) };
		return parseFileContent(std::move(ss));
	}

	/**
	 * @function writeToFile(const std::string&, Alchemy::IngrList&&)
	 * @brief Write an ingredient list to a file.
	 * @param filename	- Target file.
	 * @param ingr		- rvalue ref of an IngrList instance.
	 * @returns bool
	 *			true	- Success.
	 *			false	- Failed to write to file.
	 */
	inline bool writeToFile(const std::string& filename, Alchemy::IngrList&& ingr)
	{
		std::stringstream ss;

		for ( auto& it : ingr ) {
			ss << it._name << "\n{\n";
			for ( auto& fx : it._effects ) {
				ss << fx._name << "\t\t= " << fx._magnitude;
			}
			ss << "}\n";
		}

		return file::write(filename, ss, false);
	}

	/**
	 * @function validate_file(const std::string&)
	 * @brief Predefined usage, made for the "--validate" commandline option. Not for use anywhere else.
	 * @param filename	- Target filename to attempt loading
	 * @returns int
	 *			0 - Success.
	 *			1 - Failure.
	 */
	int validate_file(const std::string& filename)
	{
		try {
			const auto registry{ loadFromFile(filename) };
			if ( !registry.empty() ) {
				std::cout << sys::msg << "File validation succeeded." << std::endl;
				return 0;
			}
			else throw std::exception();
		} catch ( ... ) {
			std::cout << sys::error << "File validation failed! (Does the file exist, and contain valid data?)" << std::endl;
			return -1;
		}
	}

	template<class T>
	T helper(const T& v, const T& o)
	{
		if ( v != static_cast<T>( 0 ) )
			return v;
		return o;
	}

	GameSettings read_ini(const std::string& filename)
	{
		file::xINI ini{ filename };
		return GameSettings(helper(str::stod(ini.getv("GMST", "fAlchemyIngredientInitMult")), 3.0), helper(str::stod(ini.getv("GMST", "fAlchemySkillFactor")), 3.0), helper(str::stoui(ini.getv("AV", "alchemy_skill")), 15u), helper(str::stoui(ini.getv("DEFAULTS", "duration")), 1u));
	}

	bool write_ini(const std::string& filename, const GameSettings& gs = _internal::_DEFAULT_GAMESETTINGS, const bool append = false)
	{
		return file::write(filename, std::stringstream{ "[GMST]\nfAlchemyIngredientInitMult\t= " + std::to_string(gs._fAlchemyIngredientInitMult) + "\nfAlchemySkillFactor\t= " + std::to_string(gs._fAlchemySkillFactor) + "\n\n[AV]\nalchemy_skill\t= " + std::to_string(gs._alchemy_skill) + "\n\n[DEFAULTS]\nduration\t= " + std::to_string(gs._duration) }, append);
	}
}
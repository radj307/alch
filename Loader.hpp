#pragma once
#include <algorithm>
#include <file.h>
#include "Alchemy.hpp"
#include "GameSettings.hpp"

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
	 * @function loadFromFile(const std::string&)
	 * @brief Load an alchemy effect list & ingredient list from a file.
	 * @param filename	- Name of target file.
	 * @returns std::pair<Alchemy::EffectList, Alchemy::IngrList>
	 */
	inline std::pair<Alchemy::EffectList, Alchemy::IngrList> loadFromFile(const std::string& filename)
	{
		Alchemy::IngrList ingredients;
		Alchemy::EffectList effects;
		auto ss{ file::read(filename) };

		if ( ss.fail() ) throw std::exception(( std::string("Couldn't open file: \'") + filename + std::string("\'") ).c_str());

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
					for ( auto& it : tmp_fx )
						effects.insert(it._name);
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

		return { effects, ingredients };
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

	inline bool write_ini(const std::string& filename, const GameSettings& gs)
	{
		if ( !filename.empty() ) {
			std::stringstream sb;

			sb << "[constants]\n";
			sb << "fAlchemyIngredientInitMult\t= " << gs._fAlchemyIngredientInitMult;
			sb << "fAlchemySkillFactor\t= " << gs._fAlchemySkillFactor;
			sb << "\n[alchemy]\n";
			sb << "alchemy_skill\t= " << gs._alchemy_skill;
			sb << "default_duration\t= " << gs._duration;

			return file::write(filename, std::move(sb), false);
		}
		return false;
	}

	inline GameSettings load_ini(const std::string& filename)
	{
		GameSettings gs;
		if ( file::exists(filename) ) {
			file::xINI ini(filename);
			if ( const auto val{ str::stod(ini.getv("constants", "fAlchemyIngredientInitMult")) }; val != 0.0 )
				gs._fAlchemyIngredientInitMult = val;
			if ( const auto val{ str::stod(ini.getv("constants", "fAlchemySkillFactor")) }; val != 0.0 )
				gs._fAlchemySkillFactor = val;
			if ( const auto val{ str::stoui(ini.getv("alchemy", "alchemy_skill")) }; val != 0u )
				gs._alchemy_skill = val;
			if ( const auto val{ str::stoui(ini.getv("alchemy", "default_duration")) }; val != 0u)
				gs._duration = val;
		}
		return std::move(gs);
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
			if ( !registry.first.empty() && !registry.second.empty() ) {
				std::cout << sys::msg << "File validation succeeded." << std::endl;
				return 0;
			}
			else throw std::exception();
		} catch ( ... ) {
			std::cout << sys::error << "File validation failed! (Does the file exist, and contain valid data?)" << std::endl;
			return -1;
		}
	}
}
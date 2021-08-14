#pragma once
#include <iostream>
#include <algorithm>
#include <file.h>
#include <xINI.hpp>
#include <reparse.hpp>
#include <sysapi.h>
#include "Alchemy.hpp"
/**
 * @namespace caco_alch
 * @brief Contains everything used by the caco-alch project.
 */
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

	inline Alchemy::IngrList parseFileContent(std::stringstream&& ss)
	{
		const auto find_elem{ [](reparse::Elem::Cont traits, const std::string& name) -> std::string {
			const auto pos{ std::find_if(traits.begin(), traits.end(), [&name](reparse::Elem::Cont::value_type v) { return str::tolower(v.name()) == name; }) };
			if ( pos != traits.end() && pos->isVar() )
				return pos->value();
			return { };
		} };
		const auto get_fx{ [&find_elem] (reparse::Elem::Cont::value_type& elem)->std::array<Effect, 4> {
			if ( elem.isVar() ) throw std::exception("Unrecognized File Format");
			const auto vec{ elem.getVec() };
			std::array<Effect, 4> arr;
			for ( size_t i{ 0 }; i < vec.size() && i < 4u; ++i ) {
				if ( vec.at(i).isVar() ) throw std::exception("Unrecognized File Format");
				if ( const auto traits{ vec.at(i).getVec() }; traits.size() >= 2 ) {
					const double mag{ str::stod(find_elem(traits, "magnitude")) };
					const unsigned dur{ str::stoui(find_elem(traits, "duration")) };
					arr[i] = Effect{ vec.at(i).name(), mag, dur };
				}
			}
			return arr;
		} };
		Alchemy::IngrList ingredients;
		const auto push{ [&ingredients](const std::string& name, const std::array<Effect, 4>& fx) {
			for ( auto& it : ingredients )
				if ( it._name == name )
					return false;
			ingredients.push_back({ name, fx });
			return true;
		} };
		for ( auto& elem : reparse::parse(std::forward<std::stringstream>(ss)) ) {
			if ( !push(elem.name(), get_fx(elem)) ) {
#ifdef ENABLE_DEBUG
				std::cout << sys::warn << "Found duplicate element: \'" << elem.name() << '\'' << std::endl;
#endif
			}
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
		auto buffer{ file::read(filename) };
		if ( buffer.fail() ) throw std::exception(( "Couldn't find \'" + filename + "\'" ).c_str());
		return parseFileContent(std::move(buffer));
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
		return !loadFromFile(filename).empty();
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
		return GameSettings(helper(str::stod(ini.getv("GMST", "fAlchemyIngredientInitMult")), 3.0), helper(str::stod(ini.getv("GMST", "fAlchemySkillFactor")), 3.0), helper(str::stoui(ini.getv("AV", "alchemy_skill")), 15u));
	}

	bool write_ini(const std::string& filename, const GameSettings& gs = _DEFAULT_GAMESETTINGS, const bool append = false)
	{
		return file::write(filename, std::stringstream{ "[GMST]\nfAlchemyIngredientInitMult\t= " + std::to_string(gs._fAlchemyIngredientInitMult) + "\nfAlchemySkillFactor\t= " + std::to_string(gs._fAlchemySkillFactor) + "\n\n[AV]\nalchemy_skill\t= " + std::to_string(gs._alchemy_skill) + "\n\n[DEFAULTS]\n" }, append);
	}
}
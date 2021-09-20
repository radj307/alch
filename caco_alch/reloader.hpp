#pragma once
#include <algorithm>
#include <file.h>
#include <reparse.hpp>
#include <TermAPI.hpp>
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
	 * @returns std::string
	 */
	inline std::string strip_line(std::string str)
	{
		if ( const auto dPos{ str.find_first_of("#;") }; !str.empty() && dPos != std::string::npos ) // remove comments first
			str.erase(dPos + str.begin(), str.end());
		if ( const auto first{ str.find_first_not_of(" \t\r\n\v") }; !str.empty() && first != std::string::npos )
			str.erase(std::remove_if(str.begin(), str.begin() + first, isspace), str.begin() + first); // remove from beginning of string to 1 before first non-whitespace char
		else return{ };
		if ( const auto last{ str.find_last_not_of(" \t\r\n\v") }; !str.empty() && last != std::string::npos )
			str.erase(std::remove_if(last + str.begin(), str.end(), isspace), str.end()); // remove from 1 after last non-whitespace char to the end of the string
		return str;
	}

	/**
	 * @brief Uses the reparser.hpp header to parse a given stringstream into an IngrList
	 * @param ss	- Stringstream to parse, should contain data in valid format from an ingredient cache.
	 * @returns IngrList
	 */
	inline IngrList parseFileContent(std::stringstream& ss)
	{
		constexpr auto get_fx{ [] (reparse::Elem::Cont::value_type& elem) -> std::array<Effect, 4> {
			constexpr auto find_var{ [](reparse::Elem::Cont traits, const std::string& name) -> std::string {
				if ( const auto pos{ std::find_if(traits.begin(), traits.end(), [&name](const reparse::Elem::Cont::value_type& v) { return str::tolower(v.name()) == name; }) }; pos != traits.end() && pos->isVar() )
					return pos->value();
				return { };
			} };
			if ( elem.isVar() ) throw std::exception("Unrecognized File Format");
			const auto vec{ elem.getVec() };
			std::array<Effect, 4> arr;
			for ( size_t i{ 0 }; i < vec.size() && i < 4u; ++i ) {
				if ( vec.at(i).isVar() ) throw std::exception("Unrecognized File Format");
				if ( const auto traits{ vec.at(i).getVec() }; traits.size() >= 2 ) {
					const double mag{ str::stod(find_var(traits, "magnitude")) };
					const unsigned dur{ str::stoui(find_var(traits, "duration")) };
					const auto KWDA{ [&traits]() -> KeywordList {
						KeywordList keywords;
						for ( auto& it : traits )
							if ( !it.isVar() )
								for ( auto& kywd : it.getVec() )
									if ( kywd.isVar() )
										keywords.insert(static_cast<const Keyword>(kywd.value()));
						return keywords;
					}() };
					arr[i] = Effect{ vec.at(i).name(), mag, dur, KWDA };
				}
			}
			return arr;
		} };
		IngrList ingredients;
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
				std::cout << sys::term::warn << "Found duplicate element: \'" << elem.name() << '\'' << std::endl;
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
	inline IngrList loadFromFile(const std::string& filename)
	{
		auto buffer{ file::read(filename) };
		if ( buffer.fail() ) throw std::exception(( "Couldn't find \'" + filename + "\'" ).c_str());
		return parseFileContent(buffer);
	}

	/**
	 * @function writeToFile(const std::string&, Alchemy::IngrList&&)
	 * @brief Write an ingredient list to a file.
	 * @param filename	- Target file.
	 * @param ingr		- rvalue ref of an IngrList instance.
	 * @param append	- When true, does not overwrite file contents with ingredient list, instead appends to file.
	 * @returns bool
	 *			true	- Success.
	 *			false	- Failed to write to file.
	 */
	inline bool writeToFile(const std::string& filename, IngrList&& ingr, const bool append = false)
	{
		std::stringstream ss;

		for ( auto& it : ingr ) {
			ss << it._name << "\n{\n";
			for ( auto& fx : it._effects ) {
				ss << '\t' << fx._name << "\t\t= " << fx._magnitude << '\n';
			}
			ss << "}\n";
		}

		return file::write(filename, ss, append);
	}

	/**
	 * @function validate_file(const std::string&)
	 * @brief Predefined usage, made for the "--validate" commandline option. Not for use anywhere else.
	 * @param filename	- Target filename to attempt loading
	 * @returns int
	 *			0 - Success.
	 *			1 - Failure.
	 */
	inline bool validate_file(const std::string& filename)
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
}
#pragma once
#include <fileio.hpp>
#include <str.hpp>
#include <TermAPI.hpp>

#include "Alchemy.hpp"
#include "reparse.hpp"

#include <algorithm>
/**
 * @namespace caco_alch
 * @brief Contains everything used by the caco-alch project.
 */
namespace caco_alch {
	/**
	 * @brief Uses the reparser.hpp header to parse a given stringstream into an IngrList
	 * @param ss	- Stringstream to parse, should contain data in valid format from an ingredient cache.
	 * @returns IngrList
	 */
	inline IngrList parseFileContent(std::stringstream& ss)
	{
		constexpr auto get_fx{ [] (reparse::Elem::Cont::value_type& elem)->std::array<Effect, 4> {
			constexpr auto find_var{ [](reparse::Elem::Cont traits, const std::string& name) -> std::string {
				if (const auto pos{ std::find_if(traits.begin(), traits.end(), [&name](const reparse::Elem::Cont::value_type& v) { return str::tolower(v.name()) == name; }) }; pos != traits.end() && pos->isVar())
					return pos->value();
				return {};
			} };
			if (elem.isVar()) throw make_exception("Unrecognized File Format");
			const auto vec{ elem.getVec() };
			std::array<Effect, 4> arr;
			for (size_t i{ 0 }; i < vec.size() && i < 4u; ++i) {
				if (vec.at(i).isVar()) throw make_exception("Unrecognized File Format");
				if (const auto traits{ vec.at(i).getVec() }; traits.size() >= 2) {
					const double mag{ str::stod(find_var(traits, "magnitude")) };
					const unsigned dur{ str::stoui(find_var(traits, "duration")) };
					const auto KWDA{ [&traits]() -> KeywordList {
						KeywordList keywords;
						for (auto& it : traits)
							if (!it.isVar())
								for (auto& kywd : it.getVec())
									if (kywd.isVar())
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
			for (auto& it : ingredients)
				if (it._name == name)
					return false;
			ingredients.push_back({ name, fx });
			return true;
		} };
		for (auto& elem : reparse::parse(std::forward<std::stringstream>(ss))) {
			if (!push(elem.name(), get_fx(elem))) {
			#ifdef ENABLE_DEBUG
				std::cout << term::warn << "Found duplicate element: \'" << elem.name() << '\'' << std::endl;
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
	inline IngrList loadFromFile(const std::filesystem::path& filename)
	{
		auto buffer{ file::read(filename.generic_string()) };
		if (buffer.fail()) throw make_exception("Couldn't find \"", filename.generic_string(), "\"");
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

		for (auto& it : ingr) {
			ss << it._name << "\n{\n";
			for (auto& fx : it._effects) {
				ss << '\t' << fx._name << "\t\t= " << fx._magnitude << '\n';
			}
			ss << "}\n";
		}

		return file::write(filename, ss.rdbuf(), append);
	}
}
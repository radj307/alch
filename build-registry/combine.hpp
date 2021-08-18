#pragma once
#include <iostream>
#include <reloader.hpp>

namespace build_registry {
	using namespace caco_alch;

	/**
	 * @function parseFileContent(std::stringstream&)
	 * @brief Parse a stringstream into a sorted effect list.
	 * @param ss	- Stringstream with file contents.
	 * @returns SortedEffectList
	 */
	inline SortedEffectList parseFileContent(std::stringstream& ss)
	{
		SortedEffectList fx;
		constexpr auto get_kwda{ [](const reparse::Elem::Cont& sub) -> KeywordList {
			KeywordList KWDA;
			for ( auto& it : sub )
				if ( it.isVar() )
					KWDA.insert({ it.value(), it.name() });
			return KWDA;
		} };
		const auto push{ [&fx](const std::string& name, const KeywordList& keywords ) {
			const auto [existing, state]{ fx.insert(Effect(name, keywords)) };
			if ( !state ) { // if insert failed
				auto elem { fx.extract(existing) }; // extract the existing element
				for ( auto& KYWD : keywords ) // push all keywords to existing element's keywords list
					elem.value()._keywords.insert(KYWD);
				fx.insert(std::move(elem)); // insert existing element back into set
			}
		} };
		for ( auto& elem : reparse::parse(std::forward<std::stringstream>(ss)) )
			if ( !elem.isVar() )
				push(elem.name(), get_kwda(elem.getVec()));
			else
				std::cout << sys::warn << "Found invalid element: \"" << elem.name() << "\"\n";
		return fx;
	}

	/**
	 * @function loadFromFile(const std::string&)
	 * @brief Wrapper for the parseFileContent() function that
	 * @param filename	- Target filename.
	 * @returns SortedEffectList
	 */
	inline SortedEffectList loadFromFile(const std::string& filename)
	{
		std::stringstream buffer{ file::read(filename) };
		if ( buffer.fail() ) throw std::exception(("Couldn't find \'" + filename + "\'").c_str());
		return parseFileContent(buffer);
	}

	/**
	 * @function getKeywords(const SortedEffectList&, const Effect&)
	 * @brief Retrieve the keyword list for a given effect.
	 * @param KYWD_REGISTRY	- The keyword registry to use as a lookup table.
	 * @param effect		- The effect to search for.
	 * @returns KeywordList
	 */
	inline KeywordList getKeywords(const SortedEffectList& KYWD_REGISTRY, const Effect& effect)
	{
		for ( auto& it : KYWD_REGISTRY )
			if ( str::tolower(it._name) == str::tolower(effect._name) )
				return it._keywords;
		return { };
	}

	/**
	 * @function merge_effect_keywords(IngrList&, SortedEffectList&&)
	 * @brief Merges an ingredient list with a keyword registry
	 * @param ingr	- The ingredient registry object.
	 * @param fx	- The effect-keyword registry object.
	 * @returns IngrList
	 */
	inline IngrList merge_effect_keywords(IngrList& ingr, SortedEffectList&& fx)
	{
		for ( auto& it : ingr )
			for ( auto& effect : it._effects )
				if ( const auto KYWD{ getKeywords(std::forward<SortedEffectList>(fx), effect) }; !KYWD.empty() )
					effect._keywords = KYWD;
			#ifdef ENABLE_DEBUG
				else
					std::cout << sys::debug << "No keywords for effect: \"" << effect._name << "\"\n";
			#endif
		return ingr;
	}
}
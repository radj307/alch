#pragma once
#include <reloader.hpp>

namespace build_registry {
	using namespace caco_alch;

	inline EffectList parseFileContent(std::stringstream& ss)
	{
		EffectList fx;
		constexpr auto get_kywd{ [](const reparse::Elem::Cont& sub) -> KeywordList {
			KeywordList kywd;
			for ( auto& it : sub )
				if ( it.isVar() )
					kywd.push_back({ it.value(), it.name() });
			return kywd;
		} };
		const auto push{ [&fx](const std::string& name, const KeywordList& keywords ) -> bool {
			for ( auto& it : fx )
				if ( it._name == name )
					return false;
			fx.push_back(Effect(name, keywords));
			return true;
		} };
		for ( auto& elem : reparse::parse(std::forward<std::stringstream>(ss)) ) {
			if ( !elem.isVar() )
				if ( !push(elem.name(), get_kywd(elem.getVec())) ) {
				#ifdef ENABLE_DEBUG
					std::cout << sys::warn << "Found duplicate effect: \'" << elem.name() << '\'' << std::endl;
				#endif
				}
		}
		return fx;
	}

	inline EffectList loadFromFile(const std::string& filename)
	{
		std::stringstream buffer{ file::read(filename) };
		if ( buffer.fail() ) throw std::exception(("Couldn't find \'" + filename + "\'").c_str());
		return parseFileContent(buffer);
	}

	inline KeywordList getKeywords(const EffectList& KYWD_REGISTRY, const Effect& effect)
	{
		for ( auto& it : KYWD_REGISTRY )
			if ( it._name == effect._name )
				return it._keywords;
		return { };
	}

	inline IngrList merge_effect_keywords(IngrList& ingr, EffectList&& fx)
	{
		for ( auto& it : ingr )
			for ( auto& effect : it._effects )
				if ( const auto KYWD{ getKeywords(std::forward<EffectList>(fx), effect) }; !KYWD.empty() )
					effect._keywords = KYWD;
			#ifdef ENABLE_DEBUG
				else
					std::cout << sys::debug << "No keywords for effect: \"" << effect._name << "\"\n";
			#endif
		return ingr;
	}
}
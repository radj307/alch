#pragma once
#include <vector>
#include <set>
#include "Effect.hpp"
#include "Ingredient.hpp"
#include "Keyword.hpp"
namespace caco_alch {
	using KeywordList = std::set<Keyword, _internal::less<Keyword>>;
	using EffectList = std::vector<Effect>;
	using SortedEffectList = std::set<Effect, _internal::less<Effect>>;
	using IngrList = std::vector<Ingredient>;
	using SortedIngrList = std::set<Ingredient*, _internal::less<Ingredient>>;

	inline SortedIngrList getSortedList(IngrList& ingr, const std::function<bool(const Ingredient&)>& pred)
	{
		SortedIngrList ret;
		for (auto& it : ingr)
			if (pred(it))
				ret.insert(&it);
		return ret;
	}
	inline SortedIngrList getSortedList(IngrList& ingr, const std::function<bool(const std::array<Effect, 4>&)>& pred)
	{
		SortedIngrList ret;
		for (auto& it : ingr)
			if (pred(it._effects))
				ret.insert(&it);
		return ret;
	}
}
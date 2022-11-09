#pragma once
#include "Ingredient.hpp"

#include <fileio.hpp>

#include <algorithm>
#include <filesystem>

namespace alchlib2 {
	struct Registry {
		std::vector<Ingredient> Ingredients;

		CONSTEXPR Registry() {}

		static Registry ReadFrom(std::filesystem::path const& path)
		{
			auto ss{ file::read(path) };
			nlohmann::json j;
			ss >> j;
			return j.get<Registry>();
		}
		static bool WriteTo(std::filesystem::path const& path, const Registry& registry)
		{
			nlohmann::json j{ registry };
			std::stringstream ss;
			ss << j;
			return file::write_to(path, std::move(ss), false);
		}

		CONSTEXPR void sort(const std::function<bool(Ingredient, Ingredient)>& comp)
		{
			std::sort(Ingredients.begin(), Ingredients.end(), comp);
		}

		CONSTEXPR void remove_if(const std::function<bool(Ingredient)>& pred)
		{
			Ingredients.erase(std::remove_if(Ingredients.begin(), Ingredients.end(), pred), Ingredients.end());
		}

		CONSTEXPR Registry copy_if(const std::function<bool(Ingredient)>& pred) const
		{
			Registry tmp{};
			std::copy_if(Ingredients.begin(), Ingredients.end(), std::back_inserter(tmp.Ingredients), pred);
			return tmp;
		}

		/// @brief	Inverts the result of the given predicate before passing it to remove_if.
		CONSTEXPR void apply_inclusive_filter(const std::function<bool(Ingredient)>& pred)
		{
			remove_if([&pred](auto&& ingredient) -> bool {
				return !pred($fwd(ingredient));
			});
		}

		CONSTEXPR void apply_inclusive_filter(const std::string& search_term, const bool searchIngredients, const bool searchEffects = false, const bool searchKeywords = false)
		{
			if (!searchIngredients && !searchEffects && !searchKeywords) return;
			apply_inclusive_filter([&](auto&& ingredient) -> bool {
				if (searchIngredients && ingredient.IsSimilarTo(search_term))
					return true;
				else if (searchEffects || searchKeywords) {
					return std::any_of(ingredient.effects.begin(), ingredient.effects.end(), [&](auto&& effect) {
						if (searchEffects && effect.IsSimilarTo(search_term))
							return true;
						else if (searchKeywords) {
							return std::any_of(effect.keywords.begin(), effect.keywords.end(), [&](auto&& keyword) {
								return keyword.IsSimilarTo(search_term);
							});
						}
					});
				}
				return false;
			});
		}

		CONSTEXPR Registry copy_inclusive_filter(const std::string& search_term, const bool searchIngredients, const bool searchEffects = false, const bool searchKeywords = false) const
		{
			if (!searchIngredients && !searchEffects && !searchKeywords) return {};
			return copy_if([&](Ingredient const& ingredient) -> bool {
				return (searchIngredients && ingredient.IsSimilarTo(search_term))
					|| (searchEffects && ingredient.AnyEffectIsSimilarTo(search_term))
					|| (searchKeywords && ingredient.AnyEffectKeywordIsSimilarTo(search_term));
			});
		}

		template<var::any_same_or_convertible<std::function<bool(Ingredient)>>... TPreds>
		CONSTEXPR Registry DuplicateIfAll(const TPreds&... predicates) const
		{
			return copy_if([&predicates](Ingredient const& ingredient) -> bool {
				return (predicates(ingredient) && ...);
			});
		}
		CONSTEXPR Registry DuplicateIfAll(const std::vector<const std::function<bool(Ingredient)>&>& predicates) const
		{
			return copy_if([&predicates](Ingredient const& ingredient) -> bool {
				return std::all_of(predicates.begin(), predicates.end(), [&ingredient](auto&& pred) { return pred(ingredient); });
			});
		}

		NLOHMANN_DEFINE_TYPE_INTRUSIVE(Registry, Ingredients);
	};

}

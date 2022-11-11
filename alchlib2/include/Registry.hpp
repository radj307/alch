#pragma once
#include "Ingredient.hpp"

#include <fileio.hpp>

#include <algorithm>
#include <filesystem>

namespace alchlib2 {
	class Registry {
		using container_type = std::vector<Ingredient>;
		using const_iterator = typename std::vector<Ingredient>::const_iterator;
		using iterator = typename std::vector<Ingredient>::const_iterator;

	public:
		std::vector<Ingredient> Ingredients;

		CONSTEXPR Registry() = default;
		CONSTEXPR Registry(std::vector<Ingredient>&& ingredients) : Ingredients{ std::move(ingredients) } {}
		CONSTEXPR Registry(const std::vector<Ingredient>& ingredients) : Ingredients{ ingredients } {}

	#pragma region VectorInterface
		CONSTEXPR auto begin() const { return Ingredients.begin(); }
		CONSTEXPR auto end() const { return Ingredients.end(); }
		CONSTEXPR auto rbegin() const { return Ingredients.rbegin(); }
		CONSTEXPR auto rend() const { return Ingredients.rend(); }
		CONSTEXPR auto empty() const { return Ingredients.empty(); }
		CONSTEXPR auto size() const { return Ingredients.size(); }
		auto at(const size_t& index) const { return Ingredients.at(index); }
		auto& at(const size_t& index) { return Ingredients.at(index); }
	#pragma endregion VectorInterface

	#pragma region ReadFrom
		static Registry ReadFrom(std::filesystem::path const& path)
		{
			nlohmann::json j;
			file::read(path) >> j;
			return j.get<Registry>();
		}
	#pragma endregion ReadFrom
	#pragma region WriteTo
		static bool WriteTo(std::filesystem::path const& path, const Registry& registry)
		{
			return file::write(path, nlohmann::json{ registry });
		}
	#pragma endregion WriteTo

		CONSTEXPR void sort(const std::function<bool(Ingredient, Ingredient)>& comp)
		{
			std::sort(Ingredients.begin(), Ingredients.end(), comp);
		}

		CONSTEXPR void remove_if(const std::function<bool(Ingredient)>& pred)
		{
			Ingredients.erase(std::remove_if(Ingredients.begin(), Ingredients.end(), pred), Ingredients.end());
		}

		/// @brief	Inverts the result of the given predicate before passing it to remove_if.
		CONSTEXPR void apply_inclusive_filter(const std::function<bool(Ingredient)>& pred)
		{
			remove_if([&pred](auto&& ingredient) -> bool {
				return !pred($fwd(ingredient));
			});
		}

		CONSTEXPR Registry copy_if(const std::function<bool(Ingredient)>& pred) const
		{
			Registry tmp{};
			std::copy_if(Ingredients.begin(), Ingredients.end(), std::back_inserter(tmp.Ingredients), pred);
			return tmp;
		}

		CONSTEXPR void apply_inclusive_filter(const std::string& search_term, const bool requireExactMatch, const bool searchIngredients, const bool searchEffects = false, const bool searchKeywords = false)
		{
			if (!searchIngredients && !searchEffects && !searchKeywords) return;
			apply_inclusive_filter([&](auto&& ingredient) -> bool {
				if (searchIngredients && ingredient.IsSimilarTo(search_term, requireExactMatch))
					return true;
				else if (searchEffects || searchKeywords) {
					return std::any_of(ingredient.effects.begin(), ingredient.effects.end(), [&](auto&& effect) {
						if (searchEffects && effect.IsSimilarTo(search_term, requireExactMatch))
							return true;
						else if (searchKeywords) {
							return std::any_of(effect.keywords.begin(), effect.keywords.end(), [&](auto&& keyword) {
								return keyword.IsSimilarTo(search_term, requireExactMatch);
							});
						}
					});
				}
				return false;
			});
		}

		CONSTEXPR Registry copy_inclusive_filter(const std::string& search_term, const bool requireExactMatch, const bool searchIngredients, const bool searchEffects = false, const bool searchKeywords = false) const
		{
			if (!searchIngredients && !searchEffects && !searchKeywords) return {};
			return copy_if([&](Ingredient const& ingredient) -> bool {
				return (searchIngredients && ingredient.IsSimilarTo(search_term, requireExactMatch))
					|| (searchEffects && ingredient.AnyEffectIsSimilarTo(search_term, requireExactMatch))
					|| (searchKeywords && ingredient.AnyEffectKeywordIsSimilarTo(search_term, requireExactMatch));
			});
		}

		CONSTEXPR const_iterator find_best_fit(std::string name, const bool searchIngredients = true, const bool searchEffects = true) const
		{
			std::vector<const_iterator> partialMatches;

			name = str::tolower(name);

			if (searchIngredients && searchEffects) {
				for (auto it{ Ingredients.begin() }; it != Ingredients.end(); ++it) {
					const auto name_lc{ str::tolower(it->name) };
					if (name_lc == name)
						return it;
					else if ((name_lc.find(name) < name_lc.size()))
						partialMatches.emplace_back(it);
					else for (auto fx{ it->effects.begin() }; fx != it->effects.end(); ++fx) {
						const auto lc{ str::tolower(fx->name) };
						if (lc == name)
							return it;
						else if ((lc.find(name) < lc.size()))
							partialMatches.emplace_back(it);
					}
				}
			}
			else if (searchIngredients) {
				for (auto it{ Ingredients.begin() }; it != Ingredients.end(); ++it) {
					const auto name_lc{ str::tolower(it->name) };
					if (name_lc == name)
						return it;
					else if ((name_lc.find(name) < name_lc.size()))
						partialMatches.emplace_back(it);
				}
			}
			else if (searchEffects) {
				for (auto it{ Ingredients.begin() }; it != Ingredients.end(); ++it) {
					for (auto fx{ it->effects.begin() }; fx != it->effects.end(); ++fx) {
						const auto lc{ str::tolower(fx->name) };
						if (lc == name)
							return it;
						else if ((lc.find(name) < lc.size()))
							partialMatches.emplace_back(it);
					}
				}
			}
			else throw make_exception("Both 'searchIngredients' and 'searchEffects' were false; you can't search for nothing!");

			if (partialMatches.empty())
				return Ingredients.end();
			return partialMatches.front();
		}

		CONSTEXPR Registry find_best_fit(std::vector<std::string> const& search_terms, const bool searchIngredients = true, const bool searchEffects = true) const
		{
			Registry tmp;
			if (search_terms.empty()) return tmp;
			tmp.Ingredients.reserve(search_terms.size());
			for (const auto& it : search_terms)
				if (const auto& item{ find_best_fit(it, searchIngredients, searchEffects) }; item != Ingredients.end())
					tmp.Ingredients.emplace_back(*item);
			tmp.Ingredients.shrink_to_fit();
			return tmp;
		}

		NLOHMANN_DEFINE_TYPE_INTRUSIVE(Registry, Ingredients);
	};

}

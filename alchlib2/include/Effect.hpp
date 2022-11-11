#pragma once
#include "INamedObject.hpp"
#include "Keyword.hpp"

#include <vector>

namespace alchlib2 {
	struct Effect : INamedObject {
		float magnitude;
		unsigned duration;
		std::vector<Keyword> keywords;

		/// @brief	Null effect constructor.
		STRCONSTEXPR Effect() = default;
		STRCONSTEXPR Effect(std::string const& name, const float magnitude, const unsigned duration, const std::vector<Keyword>& keywords = {}) : INamedObject(name), magnitude{ magnitude }, duration{ duration }, keywords{ keywords } {}

		[[nodiscard]] CONSTEXPR bool IsNullEffect() const { return magnitude == -0.0f && duration == 0u; }
		[[nodiscard]] CONSTEXPR EKeywordDisposition GetDisposition() const
		{
			EKeywordDisposition val{};
			for (const auto& kywd : keywords)
				val |= kywd.disposition;
			return $c(EKeywordDisposition, GetHighestBit(val));
		}
		template<var::any_same_or_convertible<Keyword>... TKeywords> requires var::at_least_one<TKeywords...>
		[[nodiscard]] CONSTEXPR bool HasAnyKeyword(TKeywords const&... keywords) const
		{
			return std::any_of(this->keywords.begin(), this->keywords.end(), [&](auto&& kywd) { return var::variadic_or(kywd == keywords...); });
		}
		[[nodiscard]] CONSTEXPR bool HasKeywordNamed(std::string const& name) const
		{
			return std::any_of(keywords.begin(), keywords.end(), [&name](auto&& kywd) { return kywd.IsSimilarTo(name, false); });
		}

		[[nodiscard]] CONSTEXPR bool IsSimilarTo(const std::string& name, const bool requireExactMatch) const
		{
			const auto thisNameLower{ str::tolower(this->name) }, nameLower{ str::tolower(name) };
			return requireExactMatch ? thisNameLower == nameLower : thisNameLower.find(nameLower) != std::string::npos;
		}
	};
}

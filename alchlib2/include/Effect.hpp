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
		Effect() : magnitude{ -0.0f }, duration{ 0u } {}
		Effect(std::string const& name, const float magnitude, const unsigned duration, const std::vector<Keyword>& keywords = {}) : INamedObject(name), magnitude{ magnitude }, duration{ duration }, keywords{ keywords } {}

		[[nodiscard]] CONSTEXPR bool IsNullEffect() const { return magnitude == -0.0f && duration == 0u; }
		[[nodiscard]] CONSTEXPR EKeywordDisposition GetDisposition() const
		{
			EKeywordDisposition val{};
			for (const auto& kywd : keywords)
				val |= kywd.disposition;
			return val;
		}

		[[nodiscard]] CONSTEXPR bool IsSimilarTo(const std::string& name) const
		{
			const auto thisNameLower{ str::tolower(this->name) }, nameLower{ str::tolower(name) };
			return thisNameLower.find(nameLower) != std::string::npos;
		}
	};
}

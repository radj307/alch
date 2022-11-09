#pragma once
#include <alchlib2.hpp>

#include <indentor.hpp>
#include <TermAPI.hpp>
#include <color-sync.hpp>
#include <palette.hpp>

#include <ostream>

static color::sync csync;
color::palette<alchlib2::EKeywordDisposition> keywordColors{
	std::make_pair(alchlib2::EKeywordDisposition::Unknown, color::light_gray),
	std::make_pair(alchlib2::EKeywordDisposition::Neutral, color::white),
	std::make_pair(alchlib2::EKeywordDisposition::Positive, color::green),
	std::make_pair(alchlib2::EKeywordDisposition::Cure, color::light_green),
	std::make_pair(alchlib2::EKeywordDisposition::FortifyStat, color::yellow),
	std::make_pair(alchlib2::EKeywordDisposition::Negative, color::red),
	std::make_pair(alchlib2::EKeywordDisposition::InfluenceOther, color::purple),
};

inline constexpr const auto EFFECT_INDENT{ 4 };
inline constexpr const auto INGREDIENT_INDENT{ 2 };
inline constexpr const auto KEYWORD_INDENT{ 6 };

struct ObjectFormatter {
	color::setcolor searchTermHighlightColor;
	bool quiet;
	bool all;

	std::tuple<std::string, std::string, std::string> split_for_highlighter(const std::string& input, const std::string& substr) const
	{
		const auto& startingPos{ str::tolower(input).find(str::tolower(substr)) };
		if (startingPos == std::string::npos) return{ input, {}, {} };
		const auto& substrLen{ substr.length() };
		return{ input.substr(0ull, startingPos), input.substr(startingPos, substrLen), input.substr(startingPos + substrLen) };
	}

	std::string to_string(const alchlib2::Ingredient& ingredient, std::string const& search_term) const
	{
		if (!search_term.empty()) {
			const auto& [pre, highlight, post] { split_for_highlighter(ingredient.name, search_term) };

			if (!highlight.empty() || !post.empty())
				return str::stringify(csync(color::bold), pre, csync(searchTermHighlightColor), highlight, csync(color::reset), post, csync(color::no_bold));
		}
		return str::stringify(csync(color::bold), ingredient.name, csync());
	}
	std::string to_string(const alchlib2::Effect& effect, std::string const& search_term) const
	{
		if (!search_term.empty()) {
			const auto& [pre, highlight, post] { split_for_highlighter(effect.name, search_term) };

			if (!highlight.empty() || !post.empty())
				return str::stringify(csync(color::bold), pre, csync(searchTermHighlightColor), highlight, csync(color::reset), post, csync(color::no_bold));
		}
		return str::stringify(csync(color::bold), effect.name, csync());
	}

	std::ostream& print(std::ostream& os, const alchlib2::Ingredient& ingredient, const std::string& search_term = {})
	{
		os << shared::indent(INGREDIENT_INDENT) << to_string(ingredient, search_term);
		if (!quiet || all) {
			for (const auto& effect : ingredient.effects) {
				os << '\n' << shared::indent(EFFECT_INDENT) << to_string(effect, search_term);
				if (all) {
					for (const auto& keyword : effect.keywords) {
						os << '\n' << shared::indent(KEYWORD_INDENT) << keywordColors(keyword.disposition) << keyword.name << keywordColors();
					}
				}
			}
		}
		return os;
	}
};

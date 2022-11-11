#pragma once
#include <alchlib2.hpp>

#include <indentor.hpp>
#include <TermAPI.hpp>
#include <color-sync.hpp>
#include <color-palette.hpp>
#include <palette.hpp>

#include <ostream>

//enum class Colors : std::uint8_t {
//	Default,
//	IngredientName,
//	EffectName,
//	EffectMagnitude,
//	EffectDuration,
//	KeywordName,
//};

static color::sync csync;
static color::palette<alchlib2::EKeywordDisposition> keywordColors{
	std::make_pair(alchlib2::EKeywordDisposition::Unknown, color::light_gray),
	std::make_pair(alchlib2::EKeywordDisposition::Neutral, color::white),
	std::make_pair(alchlib2::EKeywordDisposition::Positive, color::green),
	std::make_pair(alchlib2::EKeywordDisposition::Cure, color::light_green),
	std::make_pair(alchlib2::EKeywordDisposition::FortifyStat, color::cyan),
	std::make_pair(alchlib2::EKeywordDisposition::Negative, color::red),
	std::make_pair(alchlib2::EKeywordDisposition::InfluenceOther, color::purple),
};

inline constexpr const auto EFFECT_INDENT{ 4 };
inline constexpr const auto INGREDIENT_INDENT{ 2 };
inline constexpr const auto KEYWORD_INDENT{ 6 };

inline constexpr const auto EFFECT_MAGNITUDE_INDENT{ 40 };
inline constexpr const auto EFFECT_DURATION_INDENT{ 6 };

struct ObjectFormatter {
	color::setcolor searchTermHighlightColor;
	bool quiet;
	bool all;

	STRCONSTEXPR ObjectFormatter(const color::setcolor& searchTermHighlightColor, const bool quiet = false, const bool all = false) : searchTermHighlightColor{ searchTermHighlightColor }, quiet{ quiet }, all{ all } {}

#	pragma region split_for_highlighter
	std::tuple<std::string, std::string, std::string> split_for_highlighter(const std::string& input, const std::string& substr) const
	{
		const auto& startingPos{ str::tolower(input).find(str::tolower(substr)) };
		if (startingPos == std::string::npos) return{ input, {}, {} };
		const auto& substrLen{ substr.length() };
		return{ input.substr(0ull, startingPos), input.substr(startingPos, substrLen), input.substr(startingPos + substrLen) };
	}
	std::tuple<std::string, std::string, std::string> split_for_highlighter(const std::string& input, const std::vector<std::string>& substrs) const
	{
		for (const auto& it : substrs) {
			if (const auto tpl{ split_for_highlighter(input, it) }; !std::get<1>(tpl).empty() || !std::get<2>(tpl).empty()) {
				return tpl;
			}
		}
		return{ input, {}, {} };
	}
#	pragma endregion split_for_highlighter
	bool do_highlight(std::string input, const std::string& search_term, const bool onlyHighlightExactMatch = false) const
	{
		return onlyHighlightExactMatch ? str::tolower(search_term) == str::tolower(input) : str::tolower(input).find(str::tolower(search_term)) != std::string::npos;
	}
	bool do_highlight(std::string input, const std::vector<std::string>& search_terms, const bool onlyHighlightExactMatch = false) const
	{
		input = str::tolower(input);
		return std::any_of(search_terms.begin(), search_terms.end(), [&](auto&& search_term) { return do_highlight(input, search_term, onlyHighlightExactMatch); });
	}

#	pragma region to_string
	template<var::any_same_or_convertible<std::string, std::vector<std::string>> TSearchTerm>
	std::string to_string(const alchlib2::Ingredient& ingredient, TSearchTerm const& search_term, const bool onlyHighlightExactMatch = false) const
	{
		if (onlyHighlightExactMatch) {
			if (do_highlight(ingredient.name, search_term, onlyHighlightExactMatch)) {
				return str::stringify(csync(color::bold), csync(searchTermHighlightColor), ingredient.name, csync(color::reset), csync(color::no_bold));
			}
		}
		else if (!search_term.empty()) {
			const auto& [pre, highlight, post] { split_for_highlighter(ingredient.name, search_term) };

			if (!highlight.empty() || !post.empty())
				return str::stringify(csync(color::bold), pre, csync(searchTermHighlightColor), highlight, csync(color::reset), post, csync(color::no_bold));
		}
		return str::stringify(csync(color::bold), ingredient.name, csync());
	}
	template<var::any_same_or_convertible<std::string, std::vector<std::string>> TSearchTerm>
	std::string to_string(const alchlib2::Effect& effect, TSearchTerm const& search_term, const bool onlyHighlightExactMatch = false) const
	{
		const auto& disposition{ effect.GetDisposition() };
		const auto magnitudeStr{ (effect.magnitude == 0.0f || effect.magnitude == -0.0f) ? "" : str::stringify(effect.magnitude) };
		const auto durationStr{ (effect.duration == 0) ? "" : str::stringify(effect.duration, 's') };
		const bool printMagnitude{ !magnitudeStr.empty() };
		const bool printDuration{ !durationStr.empty() };
		std::stringstream ss;

		if (onlyHighlightExactMatch) {
			if (do_highlight(effect.name, search_term, onlyHighlightExactMatch))
				ss << csync(searchTermHighlightColor) << effect.name << csync();
			else
				ss << keywordColors(disposition) << effect.name << keywordColors();
		}
		else {
			const auto& [pre, highlight, post] { split_for_highlighter(effect.name, search_term) };

			if (pre.size() != effect.name.size())
				ss << keywordColors(disposition) << pre << keywordColors() << csync(searchTermHighlightColor) << highlight << csync() << keywordColors(disposition) << post << keywordColors();
			else
				ss << keywordColors(disposition) << effect.name << keywordColors();
		}

		if (printMagnitude || printDuration) {
			ss << shared::indent(EFFECT_MAGNITUDE_INDENT, effect.name.size());
			if (printMagnitude)
				ss << csync(color::intense_magenta) << magnitudeStr << csync();
			if (printDuration)
				ss << shared::indent(EFFECT_DURATION_INDENT, magnitudeStr.size()) << csync(color::cyan) << durationStr << csync();
		}

		return ss.str();
	}
#	pragma endregion to_string

#	pragma region print
	template<var::any_same_or_convertible<std::string, std::vector<std::string>> TSearchTerm = std::string>
	std::ostream& print(std::ostream& os, const alchlib2::Effect& effect, const TSearchTerm& search_term = {}, const bool& onlyHighlightExactMatch = false)
	{
		os << shared::indent(EFFECT_INDENT) << to_string(effect, search_term, onlyHighlightExactMatch);
		if (all) {
			for (const auto& keyword : effect.keywords) {
				os << '\n' << shared::indent(KEYWORD_INDENT) << keywordColors(keyword.disposition) << keyword.name << keywordColors();
			}
		}
		return os;
	}
	template<var::any_same_or_convertible<std::string, std::vector<std::string>> TSearchTerm = std::string>
	std::ostream& print(std::ostream& os, const alchlib2::Ingredient& ingredient, TSearchTerm const& search_term = {}, const bool onlyHighlightExactMatch = false)
	{
		os << shared::indent(INGREDIENT_INDENT) << to_string(ingredient, search_term, onlyHighlightExactMatch);
		if (quiet) {
			for (const auto& effect : ingredient.effects) {
				if (do_highlight(effect.name, search_term, onlyHighlightExactMatch)) {
					os << '\n' << shared::indent(EFFECT_INDENT) << to_string(effect, search_term, onlyHighlightExactMatch);
					if (all) {
						for (const auto& keyword : effect.keywords) {
							os << '\n' << shared::indent(KEYWORD_INDENT) << keywordColors(keyword.disposition) << keyword.name << keywordColors();
						}
					}
				}
			}
		}
		else if (!quiet || all) {
			for (const auto& effect : ingredient.effects) {
				os << '\n' << shared::indent(EFFECT_INDENT) << to_string(effect, search_term, onlyHighlightExactMatch);
				if (all) {
					for (const auto& keyword : effect.keywords) {
						os << '\n' << shared::indent(KEYWORD_INDENT) << keywordColors(keyword.disposition) << keyword.name << keywordColors();
					}
				}
			}
		}
		return os;
	}
#	pragma endregion print
};

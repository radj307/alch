#pragma once
#include "ObjectBase.hpp"
#include "Keyword.hpp"

#include <var.hpp>
#include <str.hpp>

#include <string>
#include <set>

namespace caco_alch {
	using KeywordList = std::set<Keyword, _internal::less<Keyword>>;

	/**
	 * @struct Effect
	 * @brief Stores information about potion effects.
	 */
	struct Effect : ObjectBase {
		double _magnitude;	///< @brief This effect's magnitude.
		unsigned _duration;
		KeywordList _keywords;

		Effect() : _magnitude{ -0.0 }, _duration{ 0u } {}
		Effect(const std::string& name, const double magnitude, const unsigned duration) : ObjectBase(name), _magnitude{ magnitude }, _duration{ duration } {}
		Effect(const std::string& name, const KeywordList& keywords) : ObjectBase(name), _magnitude{ -0.0 }, _duration{ 0u }, _keywords{ keywords } {}
		Effect(const std::string& name, const double magnitude, const unsigned duration, const KeywordList& keywords) : ObjectBase(name), _magnitude{ magnitude }, _duration{ duration }, _keywords{ keywords } {}
		Effect(const std::string& name, KeywordList&& keywords) : ObjectBase(name), _magnitude{ -0.0 }, _duration{ 0u }, _keywords{ std::move(keywords) } {}
		Effect(const std::string& name, const double magnitude, const unsigned duration, KeywordList&& keywords) : ObjectBase(name), _magnitude{ magnitude }, _duration{ duration }, _keywords{ std::move(keywords) } {}

		/** // Working Variadic Template Example
		 * @function hasKeyword(const T&...) const
		 * @brief Check if this effect has any of a number of keywords.
		 * @tparam ...T	- Variadic Argument.
		 * @param KWDA	- Any number of std::strings to check for.
		 * @returns bool
		 *			true	- This effect has one of the given keywords.
		 *			false	- This effect does not have any of the given keywords.
		 */
		template<typename ...T> [[nodiscard]] bool hasKeyword(const T&... KWDA) const
		{
			static_assert(sizeof...(KWDA) > 0u);
			if (_keywords.empty())
				return false;
			return var::variadic_or(std::find(_keywords.begin(), _keywords.end(), KWDA) != _keywords.end()...);
		}
		/**
		 * @function hasKeyword(const std::string& KYWD) const
		 * @brief Check if this effect has a given keyword.
		 * @returns bool
		 *			true	- This effect has the given keyword.
		 *			false	- This effect does not have the given keyword.
		 */
		[[nodiscard]] bool hasKeyword(const std::string& KYWD) const
		{
			if (_keywords.empty())
				return false;
			return std::any_of(_keywords.begin(), _keywords.end(), [&KYWD](const KeywordList::value_type& k) { return k._name == KYWD; });
		}
		/**
		 * @function hasKeyword(const KeywordList&) const
		 * @brief Check if this effect has any of a number of keywords.
		 * @param KWDA	- List of keywords to check for.
		 * @returns bool
		 *			true	- This effect has one of the given keywords.
		 *			false	- This effect does not have any of the given keywords.
		 */
		[[nodiscard]] bool hasKeyword(const KeywordList& KWDA) const
		{
			return std::any_of(KWDA.begin(), KWDA.end(), [this](const Keyword& KYWD) -> bool { return std::find(_keywords.begin(), _keywords.end(), KYWD) != _keywords.end(); });
		}
		[[nodiscard]] bool is_match(const Effect& o) const { return o._name == _name; }

		bool operator==(const Effect& o) const { return o._name == _name && o._magnitude == _magnitude; }
		bool operator!=(const Effect& o) const { return o._name != _name && o._magnitude != _magnitude; }

		bool operator==(const std::string& oname) const { return oname == _name; }
		bool operator!=(const std::string& oname) const { return oname != _name; }

		bool operator<(const Effect& o) const { return _name == o._name && _magnitude < o._magnitude; }
		bool operator>(const Effect& o) const { return _name == o._name && _magnitude > o._magnitude; }
	};

	enum class KeywordType : unsigned char {
		NEUTRAL = 0,
		NEGATIVE = 1,
		POSITIVE = 2,
	};

	// KEYWORD CHECKERS
	/**
	 * @brief Fallback method for identifying postive/negative effects that checks the effect's lowercase name.
	 * @param name_lc	- The name of an effect.
	 * @returns short
	 *\n		0		- Nothing found, effect name does not indicate either positive or negative.
	 *\n		1		- Negative word found, effect name indicates it is negative.
	 *\n		2		- Positive word found, effect name indicates it is positive.
	 */
	inline KeywordType fallbackGetKeywordType(const std::string& name_lc)
	{
		const auto matches_any{ [&name_lc] <var::same_or_convertible<std::string>... vT>(const vT&... searches) { return var::variadic_or(name_lc == searches...); } };
		const auto contains_any{ [&name_lc] <var::same_or_convertible<std::string>... vT>(const vT&... searches) { return var::variadic_or(str::pos_valid(name_lc.find(searches))...); } };
		if (matches_any("health", "stamina", "magicka", "speed", "night eye", "feather", "waterwalking", "waterbreathing", "blood") // direct matches
			|| contains_any("restore", "regenerat", "absorption", "fortif", "resist", "detect", "invisi", "cure")) // contains
			return KeywordType::POSITIVE;
		else if (matches_any("slow", "frenzy", "fear", "silence", "fatigue") // direct matches
			|| contains_any("damage", "ravage", "drain", "aversion", "paraly")) // contains
			return KeywordType::NEGATIVE;
		return KeywordType::NEUTRAL;
	}

	inline bool hasPositive(const Effect& effect) { return effect.hasKeyword(Keywords::positive) || fallbackGetKeywordType(str::tolower(effect._name)) == KeywordType::POSITIVE; }
	inline bool hasNegative(const Effect& effect) { return effect.hasKeyword(Keywords::negative) || fallbackGetKeywordType(str::tolower(effect._name)) == KeywordType::NEGATIVE; }
}
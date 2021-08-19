#pragma once
#include <string>
#include <var.hpp>
#include "ObjectBase.hpp"
#include "Keyword.hpp"

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

		/**
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
		[[nodiscard]] bool hasKeyword(const std::string& KYWD) const { if ( _keywords.empty() ) return false; return std::find_if(_keywords.begin(), _keywords.end(), [&KYWD](const KeywordList::value_type& k){ return k._name == KYWD; }) != _keywords.end(); }
		[[nodiscard]] bool is_match(const Effect& o) const { return o._name == _name; }

		bool operator==(const Effect& o) const { return o._name == _name && o._magnitude == _magnitude; }
		bool operator!=(const Effect& o) const { return o._name != _name && o._magnitude != _magnitude; }

		bool operator<(const Effect& o) const { return _name == o._name && _magnitude < o._magnitude; }
		bool operator>(const Effect& o) const { return _name == o._name && _magnitude > o._magnitude; }
	};

	static bool hasPositive(const Effect& effect)
	{
		using namespace Keywords;
		return effect.hasKeyword(
				KYWD_Beneficial,
				KYWD_RestoreHealth,
				KYWD_FortifyHealth,
				KYWD_FortifyRegenHealth,
				KYWD_RestoreStamina,
				KYWD_FortifyStamina,
				KYWD_FortifyRegenStamina,
				KYWD_RestoreMagicka,
				KYWD_FortifyMagicka,
				KYWD_FortifyRegenMagicka
		);
	}
	static bool hasNegative(const Effect& effect)
	{
		using namespace Keywords;
		return effect.hasKeyword(
				KYWD_Harmful,
				KYWD_DamageHealth,
				KYWD_DamageStamina,
				KYWD_DamageMagicka
		);
	}
}
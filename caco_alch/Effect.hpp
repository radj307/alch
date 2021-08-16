#pragma once
#include <vector>
#include <string>
#include "ObjectBase.hpp"
#include "Keyword.hpp"

namespace caco_alch {
	using KeywordList = std::vector<Keyword>;
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

		[[nodiscard]] bool hasKeyword(const Keyword& KYWD) const { if ( _keywords.empty() ) return false; return std::find(_keywords.begin(), _keywords.end(), KYWD) != _keywords.end(); }
		[[nodiscard]] bool is_match(const Effect& o) const { return o._name == _name; }

		bool operator==(const Effect& o) const { return o._name == _name && o._magnitude == _magnitude; }
		bool operator!=(const Effect& o) const { return o._name != _name && o._magnitude != _magnitude; }

		bool operator<(const Effect& o) const { return _name == o._name && _magnitude < o._magnitude; }
		bool operator>(const Effect& o) const { return _name == o._name && _magnitude > o._magnitude; }
	};
}
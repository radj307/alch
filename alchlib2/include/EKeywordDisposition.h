#pragma once
#include <sysarch.h>

#include <ostream>
#include <nlohmann/json.hpp>

namespace alchlib2 {
	// this matches the definition in the alch-registry-generator project
	enum class EKeywordDisposition : std::uint8_t {
		Unknown = 0,
		Neutral = 1,
		Positive = 2,
		Cure = 4,
		FortifyStat = 8,
		Negative = 16,
		InfluenceOther = 32,
	};
	$make_bitfield_operators(EKeywordDisposition, std::uint8_t);

	inline CONSTEXPR std::uint8_t GetHighestBit(EKeywordDisposition const& disposition)
	{
		auto n{ $c(std::uint8_t, disposition) };
		n |= (n >> 1);
		n |= (n >> 2);
		n |= (n >> 4);
		return n ^ (n >> 1);
	}
}

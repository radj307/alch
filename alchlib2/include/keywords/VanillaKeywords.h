#pragma once
#include "../Keyword.hpp"

namespace alchlib2::keywords {
	inline static const Keyword MagicAlchBeneficial{ "MagicAlchBeneficial", "0F8A4E", (EKeywordDisposition)2 };
	inline static const Keyword MagicAlchRestoreHealth{ "MagicAlchRestoreHealth", "042503", (EKeywordDisposition)2 };
	inline static const Keyword MagicAlchRestoreStamina{ "MagicAlchRestoreStamina", "042504", (EKeywordDisposition)2 };
	inline static const Keyword MagicAlchRestoreMagicka{ "MagicAlchRestoreMagicka", "042508", (EKeywordDisposition)2 };
	inline static const Keyword MagicAlchHarmful{ "MagicAlchHarmful", "042509", (EKeywordDisposition)16 };
	inline static const Keyword MagicAlchDurationBased{ "MagicAlchDurationBased", "0F8A4F", (EKeywordDisposition)1 };
}

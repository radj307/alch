#pragma once
#include <TermAPI.hpp>
#include <palette.hpp>

#include "../alchlib2/include/Potion.hpp"
#include "../alchlib2/include/GameSetting.hpp"

static struct {
	color::palette<alchlib2::EKeywordDisposition> keyword_disposition{
		std::make_pair(alchlib2::EKeywordDisposition::Unknown, color::setcolor(color::gray)),
		std::make_pair(alchlib2::EKeywordDisposition::Neutral, color::setcolor::white),
		std::make_pair(alchlib2::EKeywordDisposition::Positive, color::setcolor::intense_green),
		std::make_pair(alchlib2::EKeywordDisposition::Negative, color::setcolor::white),
		std::make_pair(alchlib2::EKeywordDisposition::Cure, color::setcolor::green),
		std::make_pair(alchlib2::EKeywordDisposition::InfluenceOther, color::setcolor::yellow),
	};
} Colors;



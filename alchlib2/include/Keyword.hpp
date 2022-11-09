#pragma once
#include "INamedObject.hpp"
#include "EKeywordDisposition.h"

#include <strconv.hpp>

#include <compare>
#include <map>

namespace alchlib2 {
	struct Keyword : INamedObject {
		std::string formID;
		EKeywordDisposition disposition;

		STRCONSTEXPR Keyword() {}
		STRCONSTEXPR Keyword(std::string const& name, std::string const& formID, EKeywordDisposition const& disposition = EKeywordDisposition::Unknown) : INamedObject(name), formID{ formID }, disposition{ disposition } {}

		friend STRCONSTEXPR bool operator==(Keyword const& l, Keyword const& r) noexcept
		{
			return str::tolower(l.name) == str::tolower(r.name) && str::tolower(l.formID) == str::tolower(r.formID);
		}
		friend STRCONSTEXPR bool operator!=(Keyword const& l, Keyword const& r) noexcept
		{
			return str::tolower(l.name) != str::tolower(r.name) || str::tolower(l.formID) != str::tolower(r.formID);
		}
		friend STRCONSTEXPR bool operator==(Keyword const& l, std::string s) noexcept
		{
			s = str::tolower(s);
			return str::tolower(l.name) == s || str::tolower(l.formID) == s;
		}
		friend STRCONSTEXPR bool operator!=(Keyword const& l, std::string s) noexcept
		{
			s = str::tolower(s);
			return str::tolower(l.name) != s && str::tolower(l.formID) != s;
		}

		STRCONSTEXPR bool IsSimilarTo(const Keyword& keyword) const
		{
			const auto lc_inname{ str::tolower(keyword.name) }, lc_infid{ str::tolower(keyword.formID) }, lc_name{ str::tolower(name) }, lc_fid{ str::tolower(formID) };
			return operator==(*this, lc_inname) || operator==(*this, lc_infid) || lc_name.find(lc_inname) != std::string::npos || lc_fid.find(lc_infid) != std::string::npos;
		}
		STRCONSTEXPR bool IsSimilarTo(std::string name_or_id) const
		{
			name_or_id = str::tolower(name_or_id);
			return operator==(*this, name_or_id) || str::tolower(name).find(name_or_id) != std::string::npos || str::tolower(formID).find(name_or_id) != std::string::npos;
		}
	};
}

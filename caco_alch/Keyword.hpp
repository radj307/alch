#pragma once
#include "ObjectBase.hpp"

namespace caco_alch {
	struct Keyword : ObjectBase {
		std::string _form_id;

		Keyword(const std::string& name, const std::string& formID) : ObjectBase(name), _form_id{ formID } {}
		explicit Keyword(const std::string& name) : ObjectBase(name) {}

		bool operator==(const Keyword& o) const { return _form_id == o._form_id; }
		bool operator!=(const Keyword& o) const { return _form_id != o._form_id; }

		bool operator<(const Keyword& o) const { return _form_id < o._form_id; }
		bool operator>(const Keyword& o) const { return _form_id > o._form_id; }

		friend std::ostream& operator<<(std::ostream& os, const Keyword& kywd)
		{
			os << kywd._form_id << " = " << kywd._name;
			return os;
		}
	};
	using KeywordList = std::vector<Keyword>;

	inline std::ostream& operator<<(std::ostream& os, const KeywordList& KWDA)
	{
		const auto indentation{ std::string(2u, '\t') };
		os << indentation << "Keywords\n" << indentation << "{\n";
		for ( auto& KYWD : KWDA )
			os << indentation << '\t' << KYWD._form_id << " = " << KYWD._name << '\n';
		os << indentation << "}\n\t";
		return os;
	}
}
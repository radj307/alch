#pragma once
#include <sysarch.h>

#include <string>
#include <ostream>

namespace alchlib2 {
	struct INamedObject {
		std::string name;

		virtual ~INamedObject() = default;
	protected:
		STRCONSTEXPR INamedObject() = default;
		STRCONSTEXPR INamedObject(std::string&& name) : name{ std::move(name) } {}
		STRCONSTEXPR INamedObject(std::string const& name) : name{ name } {}

		friend std::ostream& operator<<(std::ostream& os, const INamedObject& namedObject)
		{
			return os << namedObject.name;
		}
	};
}

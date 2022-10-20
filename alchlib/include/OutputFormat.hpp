#pragma once
#include "ColorAPI.hpp"

#include <sysarch.h>
#include <INIRedux.hpp>
#include <opt3.hpp>

#include <optional>

namespace caco_alch {
	/**
	 * @struct OutputFormat
	 * @brief Contains all of the variables used within the Format struct.
	 */
	struct OutputFormat {
		bool
			_flag_quiet,
			_flag_verbose,
			_flag_exact,
			_flag_all,
			_flag_export,
			_flag_reverse,
			_flag_color,
			_flag_smart;
		size_t
			_indent,
			_precision;
		//ColorAPI ColorAPI;

		explicit OutputFormat(const bool quiet, const bool verbose, const bool exact, const bool all, const bool fileExport, const bool reverse, const bool color, const bool smart, const size_t indent, const size_t precision) ://, ColorAPI colors) :
			_flag_quiet{ quiet },
			_flag_verbose{ verbose },
			_flag_exact{ exact },
			_flag_all{ all },
			_flag_export{ fileExport },
			_flag_reverse{ reverse },
			_flag_color{ color },
			_flag_smart{ smart },
			_indent{ indent },
			_precision{ precision }
			//		ColorAPI{ std::move(colors) }
		{}
		OutputFormat(const opt3::ArgManager& args, const std::optional<file::ini::INI>& ini = std::nullopt) :
			_flag_quiet{ args.check<opt3::Flag>('q') },
			_flag_verbose{ args.check<opt3::Flag>('v') },
			_flag_exact{ args.check<opt3::Flag>('e') },
			_flag_all{ args.check<opt3::Flag>('a') },
			_flag_export{ args.check<opt3::Flag>('E') },
			_flag_reverse{ args.check<opt3::Flag>('R') },
			_flag_color{ args.check<opt3::Flag>('c') },
			_flag_smart{ args.check<opt3::Flag>('S') },
			_indent{ str::stoui(args.getv<opt3::Option>("indent"s).value_or([&ini]() -> std::string {if (ini.has_value())if (const auto value{ ini.value().getvs("format", "indent") }; !value.has_value()) return value.value(); return "2"; }())) },
			_precision{ str::stoui(args.getv<opt3::Option>("precision").value_or([&ini]() -> std::string { if (ini.has_value()) if (const auto value{ ini.value().getvs("format", "precision") }; !value.has_value()) return value.value(); return "2"; }())) } {}

		virtual ~OutputFormat() = default;
	protected:
		[[nodiscard]] bool match(const std::string& objName, const std::string& searchName) const
		{
			return _flag_exact ? (objName == searchName) : (objName == searchName || objName.find(searchName) < objName.size());
		}
	public:
		[[nodiscard]] bool quiet() const { return _flag_quiet; }
		[[nodiscard]] bool verbose() const { return _flag_verbose; }
		[[nodiscard]] bool all() const { return _flag_all; }
		[[nodiscard]] bool file_export() const { return _flag_export; }
		[[nodiscard]] bool reverse_output() const { return _flag_reverse; }
		[[nodiscard]] bool doLocalCaching() const { return _flag_smart; }
		[[nodiscard]] size_t indent() const { return _indent; }
		[[nodiscard]] size_t precision() const { return _precision; }
		[[nodiscard]] bool color() const { return _flag_color; }
	};
}
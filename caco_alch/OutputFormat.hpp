#pragma once
#include <ColorAPI.hpp>

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
		unsigned
			_indent,
			_precision;
		ColorAPI _colors;

		explicit OutputFormat(const bool quiet, const bool verbose, const bool exact, const bool all, const bool fileExport, const bool reverse, const bool color, const bool smart, const size_t indent, const size_t precision, ColorAPI colors) :
			_flag_quiet{ quiet },
			_flag_verbose{ verbose },
			_flag_exact{ exact },
			_flag_all{ all },
			_flag_export{ fileExport },
			_flag_reverse{ reverse },
			_flag_color{ color },
			_flag_smart{ smart },
			_indent{ indent },
			_precision{ precision },
			_colors{ std::move(colors) }
		{}
		OutputFormat(const opt::Params& args, const std::optional<file::ini::INI>& ini = std::nullopt) :
			_flag_quiet{ args.check('q')},
			_flag_verbose{ args.check('v')},
			_flag_exact{ args.check('e')},
			_flag_all{ args.check('a') },
			_flag_export{ args.check('E')},
			_flag_reverse{ args.check('R')},
			_flag_color{ args.check('c')},
			_flag_smart{ args.check('S')},
			_indent{ str::stoui(args.getv("indent").value_or([&ini]() -> std::string {
					if (ini.has_value())
						if (const auto value{ ini.value().getv("format", "indent")}; !value.empty())
							return value;
					return "2";
				}())) },
			_precision{ str::stoui(args.getv("precision").value_or([&ini]() -> std::string {
					if (ini.has_value())
						if (const auto value{ ini.value().getv("format", "precision") }; !value.empty())
							return value;
					return "2";
				}())) },
			_colors{ ini.has_value() ? loadColorConfig(ini.value()) : ColorAPI{ DefaultObjects._default_colors } }
		{}

		virtual ~OutputFormat() = default;

		[[nodiscard]] bool match(const std::string& objName, const std::string& searchName) const
		{
			return _flag_exact ? (objName == searchName) : (objName == searchName || objName.find(searchName) < objName.size());
		}

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
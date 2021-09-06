/**
 * @file Output.hpp
 * @brief Designed as a less confusing, more modular replacement for Format.hpp
 */
#pragma once
#include <xINI.hpp>
#include "using.h"

namespace caco_alch {
	namespace settings {
		/**
		 * @struct FileOptions
		 * @brief Contains options specified in an INI file when loaded.
		 */
		struct FileOptions {
			std::string _path;
			file::xINI _file;

			FileOptions(std::string filepath, const bool load_now = false) : _path{ std::move(filepath) }, _file{ load_now ? file::parseINI(_path) : file::xINI{} } {}
		};

		/**
		 * @struct UserOptions
		 * @brief Contains options specified on the commandline by the user.
		 */
		struct UserOptions {
			bool 
				_quiet,						///< @brief When true, the quiet flag was specified and effects should only be included in output when they were searched for.
				_verbose,					///< @brief When true, the verbose flag was specified and effect keywords are included in output. This option is implied when the export flag is true.
				_exact,						///< @brief When true, the exact flag was specified and only full name matches are allowed.
				_all,						///< @brief When true, the all flag was specified and all additional output is enabled.
				_reverse,					///< @brief When true, output ordering is reversed where possible.
				_color_allow,				///< @brief When true, colors are enabled.
				_color_allow_effect_name,	///< @brief When true, enables effect names being colorized.
				_cache_save_file;			///< @brief When true, enables writing the cache to file.
		};
		
	}
	namespace Format {
		// Templated simple operator<<
		template<class T> inline std::ostream& operator<<(std::ostream& os, const T& obj)
		{
			os << obj;
			return os;
		}
	}

} // caco_alch
#pragma once
#include <set>
#include <iomanip>
#include <sysapi.h>
#include <strconv.hpp>
#include "using.h"
#include "Ingredient.hpp"

namespace caco_alch {

	/**
	 * @struct Format
	 * @brief Provides formatting information for some output stream methods in the Alchemy class.
	 */
	struct Format {
	private:
		bool _quiet, _verbose, _exact, _all, _file_export, _reverse_output, _force_color, _allow_color_fx; // see UserAssist.hpp
		size_t _indent, _precision;
		unsigned short
			_color,									// Ingredient color
			_color_highlight{ Color::_f_yellow },	// Search string highlight color.
			_color_fx_positive{ Color::_f_green },		// Positive effect color.
			_color_fx_negative{ Color::_f_red },		// Negative effect color.
			_color_fx_mag{ Color::_f_magenta },		// Effect Magnitude color.
			_color_fx_dur{ Color::_f_cyan };		// Effect Duration color.

	public:
		/**
		 * @constructor Format(const bool = false, const bool = true, const bool = false, const bool = false, const size_t = 3u, const size_t = 2u, const unsigned short = Color::_f_white)
		 * @brief Default Constructor
		 * @param quiet				- When true, only includes effects that match part of the search string in results.
		 * @param verbose			- When true, includes additional information about an effect's magnitude and duration.
		 * @param exact				- When true, only includes exact matches in results.
		 * @param all				- When true, includes all additional information in results.
		 * @param file_export		- When true, output is formatted in the registry format to allow piping output to a file, then back in again.
		 * @param reverse_output	- When true, prints output in the opposite order as it would normally be printed. (Alphabetical by default)
		 * @param allow_color_fx	- When true, prints effect colors where possible.
		 * @param indent			- How many space characters to include before ingredient names. This is multiplied by 2 for effect names.
		 * @param precision			- How many decimal points of precision to use when outputting floating points.
		 * @param color				- General color override, changes the color of Ingredient names for search, list, and build.
		 */
		explicit Format(const bool quiet = false, const bool verbose = true, const bool exact = false, const bool all = false, const bool file_export = false, const bool reverse_output = false, const bool allow_color_fx = true, const size_t indent = 3u, const size_t precision = 2u, const unsigned short color = Color::_reset) : _quiet{ quiet }, _verbose{ verbose }, _exact{ exact }, _all{ all }, _file_export{ file_export }, _reverse_output{ reverse_output }, _force_color{ color != Color::_f_white }, _allow_color_fx{ allow_color_fx }, _indent{ indent }, _precision{ precision }, _color{ color } {}

#pragma region GETTERS
		[[nodiscard]] bool quiet() const { return _quiet; }
		[[nodiscard]] bool verbose() const { return _verbose; }
		[[nodiscard]] bool exact() const { return _exact; }
		[[nodiscard]] bool all() const { return _all; }
		[[nodiscard]] bool file_export() const { return _file_export; }
		[[nodiscard]] bool reverse_output() const { return _reverse_output; }
		[[nodiscard]] size_t indent() const { return _indent; }
		[[nodiscard]] size_t precision() const { return _precision; }
		[[nodiscard]] unsigned short color() const { return _color; }
#pragma endregion GETTERS

#pragma region SPECIAL_GETTERS
		/**
		 * @function get_tuple(const std::string&, const std::string&) const
		 * @brief Uses a string to split of a line into a tuple of strings where { <preceeding text>, <delimiter text>, <trailing text> }
		 * @param str			The line to split.
		 * @param highlight		Substr of str to separate. Must be converted to lowercase beforehand or match will not be successful.
		 * @returns std::tuple<std::string, std::string, std::string>
		 *\n		0	- Text that preceeds highlight
		 *\n		1	- highlight
		 *\n		2	- Text that supersedes highlight
		 */
		[[nodiscard]] std::tuple<std::string, std::string, std::string> get_tuple(const std::string& str, const std::string& highlight) const
		{
			if ( !str.empty() )
				if ( const auto dPos{ str::tolower(str).find(highlight) }; dPos != std::string::npos )
					return { str.substr(0, dPos), str.substr(dPos, highlight.size()), str.substr(dPos + highlight.size()) };
			return{ str, { }, { } }; // by default, return blanks for unused tuple slots. This is used to hide the text color when nothing was found.
		}

		/**
		 * @function get_fx(std::array<Effect, 4>&, const std::vector<std::string>&) const
		 * @brief Retrieve a vector of Effect pointers from an array of Effect instances.
		 * @param arr				Effect array ref.
		 * @param names_lowercase	The names to search for, must be lowercase.
		 * @returns std::vector<Effect*>
		 */
		std::vector<Effect*> get_fx(std::array<Effect, 4>& arr, const std::vector<std::string>& names_lowercase) const
		{
			std::vector<Effect*> vec;
			for ( auto it{ arr.begin() }; it != arr.end(); ++it ) {
				if ( !_quiet )
					vec.push_back(&*it);
				else if (const auto lc{str::tolower(it->_name)}; std::find_if(names_lowercase.begin(), names_lowercase.end(), [this, &lc](const std::string& name) -> bool { return lc == name || !_exact && str::pos_valid(lc.find(name)); }) != names_lowercase.end()) {
					vec.push_back(&*it);
					if ( _exact ) break;
				}
			}
			return vec;
		}
#pragma endregion SPECIAL_GETTERS

		/**
		 * @function resolveEffectColor(const Effect&) const
		 * @brief Resolve the color to use when printing an effect's name by checking its keywords.
		 * @param fx	The effect to resolve.
		 * @returns unsigned short
		 */
		[[nodiscard]] unsigned short resolveEffectColor(const Effect& fx) const
		{
			using namespace Keywords;
			if ( !_allow_color_fx || _force_color || fx._keywords.empty() )
				return _color;
			if (fx.hasKeyword(
				KYWD_Harmful,
				KYWD_DamageHealth,
				KYWD_DamageStamina,
				KYWD_DamageMagicka
			)) return _color_fx_negative; // return red for negative effects
			if (fx.hasKeyword(
				KYWD_Beneficial,
				KYWD_RestoreHealth,
				KYWD_FortifyHealth,
				KYWD_FortifyRegenHealth,
				KYWD_RestoreStamina,
				KYWD_FortifyStamina,
				KYWD_FortifyRegenStamina,
				KYWD_RestoreMagicka,
				KYWD_FortifyMagicka,
				KYWD_FortifyRegenMagicka
			)) return _color_fx_positive; // return green for positive effects
			return Color::_reset; // else return white
		}

#pragma region FSTREAM
		/**
		 * @function to_fstream(std::ostream&, const Ingredient&) const
		 * @brief Insert a registry-formatted ingredient into an output stream.
		 * @param os	- Target output stream.
		 * @param ingr	- Target ingredient.
		 * @returns std::ostream&
		 */
		static std::ostream& to_fstream(std::ostream& os, const Ingredient& ingr)
		{
			os << ingr._name << "\n{\n";
			for ( auto& fx : ingr._effects )
				os << '\t' << fx._name << "\n\t{\n\t\tmagnitude = " << fx._magnitude << "\n\t\tduration = " << fx._duration << "\n" << fx._keywords << "}\n";
			os << "}\n";
			return os;
		}

		/**
		 * @function to_fstream(std::ostream&, const SortedIngrList&) const
		 * @brief Insert a registry-formatted list of ingredients into an output stream.
		 * @param os	- Target output stream.
		 * @param ingr	- Target ingredient list.
		 * @returns std::ostream&
		 */
		std::ostream& to_fstream(std::ostream& os, const SortedIngrList& ingr) const
		{
			if ( _reverse_output )
				for ( auto it{ ingr.rbegin() }; it != ingr.rend(); ++it )
					to_fstream(os, **it);
			else
				for ( auto it{ ingr.begin() }; it != ingr.end(); ++it )
					to_fstream(os, **it);
			return os;
		}

		/**
		 * @function to_fstream(std::ostream&, const SortedIngrList&) const
		 * @brief Insert a registry-formatted list of ingredients into an output stream.
		 * @param os	- Target output stream.
		 * @param ingr	- Target ingredient list.
		 * @returns std::ostream&
		 */
		std::ostream& to_fstream(std::ostream& os, const IngrList& ingr) const
		{
			if ( _reverse_output )
				for ( auto it{ ingr.rbegin() }; it != ingr.rend(); ++it )
					to_fstream(os, *it);
			else
				for ( auto it{ ingr.begin() }; it != ingr.end(); ++it )
					to_fstream(os, *it);
			return os;
		}
#pragma endregion FSTREAM

#pragma region STREAM
		/**
		 * @function to_stream(std::ostream&, const Effect&, const std::string&, const std::string&)
		 * @brief Insert an Effect into an output stream in human-readable format.
		 * @param os				- Target output stream.
		 * @param fx				- Target Effect.
		 * @param search_str		- Search string, used to highlight searched-for strings in the output.
		 * @param indentation		- String to use as indentation before each line.
		 * @param repeatIndentation	- Repeats the indentation string this many times before the effect name.
		 * @returns std::ostream&
		 */
		std::ostream& to_stream(std::ostream& os, const Effect& fx, const std::string& search_str, const std::string& indentation, const unsigned repeatIndentation = 2u) const
		{
			const auto [pre, highlight, post]{ get_tuple(fx._name, search_str) };
			const auto fx_color{ resolveEffectColor(fx) };
			for ( auto i{ 0u }; i < repeatIndentation; ++i )
				os << indentation;
			sys::colorSet(fx_color);
			os << pre << Color::reset;
			sys::colorSet(_color_highlight);
			os << highlight;
			sys::colorSet(fx_color);
			os << post << Color::reset;
			const auto insert_num{ [&os](const std::string& num, const unsigned short color, const unsigned indent) -> unsigned {  // NOLINT(clang-diagnostic-c++20-extensions)
				if (indent > 24u)
					os << std::setw(indent + 2u) << ' ';
				else
					os << std::setw(24u - indent) << ' ';
				sys::colorSet(color);
				os << num;
				return num.size();
			} };
			auto size_factor{ fx._name.size() };
			if ( fx._magnitude > 0.0 || _all )
				size_factor = insert_num(str::to_string(fx._magnitude, _precision), _allow_color_fx ? _color_fx_mag : Color::_f_gray, size_factor) + 10u;
			if ( fx._duration > 0u || _all ) {
				insert_num(str::to_string(fx._duration, _precision), _allow_color_fx ? _color_fx_dur : Color::_f_gray, size_factor);
				os << 's';
			}
			os << Color::reset << '\n';
			return os;
		}

		/**
		 * @function to_stream(std::ostream&, Ingredient&, const std::string& = "")
		 * @brief Insert an ingredient into an output stream in human-readable format.
		 * @param os			- Target output stream.
		 * @param ingr			- Target Ingredient Ref.
		 * @param search_str	- Search string, used to highlight searched-for strings in the output.
		 * @returns std::ostream&
		 */
		std::ostream& to_stream(std::ostream& os, Ingredient& ingr, const std::string& search_str = "") const
		{
			const auto indentation{ std::string(_indent, ' ') }; // get indentation string
			const auto [pre, highlight, post]{ get_tuple(ingr._name, search_str) };
			os << indentation; // insert indentation
			sys::colorSet(_color); // set color
			os << pre << Color::reset << _color_highlight << highlight << Color::reset;
			sys::colorSet(_color); // set color
			os << post << Color::reset << '\n';
			for ( auto& fx : get_fx(ingr._effects, { search_str }) ) // iterate through this ingredient's effects, and insert them as well.
				to_stream(os, *fx, search_str, indentation);
			return os;
		}

		/**
		 * @function to_stream(std::ostream&, Potion&, const std::string& = "")
		 * @brief Insert a Potion into an output stream in human-readable format.
		 * @param os			- Target output stream.
		 * @param potion		- Target Potion Ref.
		 * @param indentation	- String to use as indentation before each line.
		 * @returns std::ostream&
		 */
		std::ostream& to_stream(std::ostream& os, const Potion& potion, const std::string& indentation) const
		{
			const auto [pre, highlight, post]{ get_tuple(potion.name(), "") };
			os << indentation; // insert indentation
			sys::colorSet(_color); // set color
			os << pre << Color::reset << _color_highlight << highlight << Color::reset;
			sys::colorSet(_color); // set color
			os << post << Color::reset << '\n';
			for ( auto& fx : potion.effects() ) // iterate through this ingredient's effects, and insert them as well.
				to_stream(os, fx, "", indentation);
			return os;
		}

		/**
		 * @function to_stream_build(std::ostream&, Ingredient&, const Potion&)
		 * @brief Insert a potion ingredient into an output stream in human-readable format.
		 * @param os			- Target output stream.
		 * @param ingr			- Target Ingredient Ref.
		 * @param potion		- Resulting potion.
		 * @returns std::ostream&
		 */
		std::ostream& to_stream_build(std::ostream& os, Ingredient& ingr, const Potion& potion) const
		{
			const auto indentation{ std::string(_indent, ' ') }; // get indentation string
			const auto names_lc{ [&potion]() -> std::vector<std::string> {
				std::vector<std::string> vec;
				for ( const auto& it : potion.effects() )
					vec.push_back(str::tolower(it._name));
				return vec;
			}() };
			os << indentation;
			sys::colorSet(_color);
			os << ingr._name << Color::reset << '\n';
			for ( auto& fx : get_fx(ingr._effects, names_lc) ) // iterate through this ingredient's effects, and insert them as well.
				to_stream(os, *fx, "", indentation);
			return os;
		}
#pragma endregion STREAM
		/**
		 * @operator()()
		 * @brief Retrieve a reference to this Format instance.
		 * @returns Format&
		 */
		Format& operator()() { return *this; }
	};
}
#pragma once
#include <set>
#include <iomanip>
#include <TermAPI.hpp>
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
		bool _quiet, _verbose, _exact, _all, _file_export, _reverse_output, _force_color, _allow_color_fx, _cache; // see UserAssist.hpp
		size_t _indent, _precision;
		short
			_color,									// Ingredient color
			_color_highlight{ color::yellow },	// Search string highlight color.
			_color_fx_positive{ color::intense_green },		// Positive effect color.
			_color_fx_negative{ color::intense_red },		// Negative effect color.
			_color_fx_neutral{ color::intense_blue },		// Negative effect color.
			_color_fx_mag{ color::intense_magenta },		// Effect Magnitude color.
			_color_fx_dur{ color::intense_cyan };		// Effect Duration color.

	public:
		/**
		 * @constructor Format(const bool = false, const bool = true, const bool = false, const bool = false, const size_t = 3u, const size_t = 2u, const unsigned short = color::_f_white)
		 * @brief Default Constructor
		 * @param quiet				- When true, only includes effects that match part of the search string in results.
		 * @param verbose			- When true, includes additional information about an effect's magnitude and duration.
		 * @param exact				- When true, only includes exact matches in results.
		 * @param all				- When true, includes all additional information in results.
		 * @param file_export		- When true, output is formatted in the registry format to allow piping output to a file, then back in again.
		 * @param reverse_output	- When true, prints output in the opposite order as it would normally be printed. (Alphabetical by default)
		 * @param allow_color_fx	- When true, prints effect colors where possible.
		 * @param use_local_cache	- When true, the alchemy instance will cache the list before parsing additional arguments.
		 * @param indent			- How many space characters to include before ingredient names. This is multiplied by 2 for effect names.
		 * @param precision			- How many decimal points of precision to use when outputting floating points.
		 * @param color				- General color override, changes the color of Ingredient names for search, list, and build.
		 */
		explicit Format(const bool quiet = false, const bool verbose = true, const bool exact = false, const bool all = false, const bool file_export = false, const bool reverse_output = false, const bool allow_color_fx = true, const bool use_local_cache = false, const size_t indent = 3u, const size_t precision = 2u, const short color = color::white) : _quiet{ quiet }, _verbose{ verbose }, _exact{ exact }, _all{ all }, _file_export{ file_export }, _reverse_output{ reverse_output }, _force_color{ color != color::white }, _allow_color_fx{ allow_color_fx }, _cache{ use_local_cache }, _indent{ indent }, _precision{ precision }, _color{ color } {}

#pragma region GETTERS
		[[nodiscard]] bool quiet() const { return _quiet; }
		[[nodiscard]] bool verbose() const { return _verbose; }
		[[nodiscard]] bool exact() const { return _exact; }
		[[nodiscard]] bool all() const { return _all; }
		[[nodiscard]] bool file_export() const { return _file_export; }
		[[nodiscard]] bool reverse_output() const { return _reverse_output; }
		[[nodiscard]] bool doLocalCaching() const { return _cache; }
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
		 * @function get_tuple(const std::string&, const std::string&) const
		 * @brief Uses a string to split of a line into a tuple of strings where { <preceeding text>, <delimiter text>, <trailing text> }
		 * @param str			The line to split.
		 * @param highlights	Substrings of str to separate. Must be converted to lowercase beforehand or match will not be successful.
		 * @returns std::tuple<std::string, std::string, std::string>
		 *\n		0	- Text that preceeds highlight
		 *\n		1	- highlight
		 *\n		2	- Text that supersedes highlight
		 */
		[[nodiscard]] std::tuple<std::string, std::string, std::string> get_tuple(const std::string& str, const std::vector<std::string>& highlights) const
		{
			if ( !str.empty() )
				for ( auto& highlight : highlights )
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
		[[nodiscard]] std::vector<Effect> get_fx(const std::array<Effect, 4>& arr, const std::vector<std::string>& names_lowercase) const
		{
			std::vector<Effect> vec;
			vec.reserve(4llu);
			for ( auto it{ arr.begin() }; it != arr.end(); ++it ) {
				if ( !_quiet )
					vec.push_back(*it);
				else if (const auto lc{str::tolower(it->_name)}; std::find_if(names_lowercase.begin(), names_lowercase.end(), [this, &lc](const std::string& name) -> bool { return lc == name || !_exact && str::pos_valid(lc.find(name)); }) != names_lowercase.end()) {
					vec.push_back(*it);
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
		[[nodiscard]] short resolveEffectColor(const Effect& fx) const
		{
			if ( !_allow_color_fx || _force_color )
				return _color;
			if (!fx._keywords.empty()) {
				if (hasNegative(fx))
					return _color_fx_negative;
				if (hasPositive(fx))
					return _color_fx_positive;
				if (!fx.hasKeyword(Keywords::KYWD_MagicInfluence))
					return _color_fx_neutral;
			}
			const auto name_lc{ str::tolower(fx._name) };
			if ( // NEGATIVE EFFECTS
				str::pos_valid(name_lc.find("damage"))
			 || str::pos_valid(name_lc.find("ravage"))
			 || str::pos_valid(name_lc.find("drain"))
			 || str::pos_valid(name_lc.find("frenzy"))
			 || str::pos_valid(name_lc.find("fear"))
			 || str::pos_valid(name_lc.find("aversion"))
				)
				return _color_fx_negative;
			if ( // POSITIVE EFFECTS
				str::pos_valid(name_lc.find("restore"))
			 || str::pos_valid(name_lc.find("fortify"))
			 || str::pos_valid(name_lc.find("resist"))
			 || str::pos_valid(name_lc.find("detect"))
			 || str::pos_valid(name_lc.find("night eye"))
			 || str::pos_valid(name_lc.find("speed"))
			)
				return _color_fx_positive;
			return color::white; // else return white
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
					to_fstream(os, *it);
			else
				for ( auto it{ ingr.begin() }; it != ingr.end(); ++it )
					to_fstream(os, *it);
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

		[[nodiscard]] std::stringstream to_fstream(const SortedIngrList& ingr) const
		{
			std::stringstream ss;
			to_fstream(ss, ingr);
			return ss;
		}
#pragma endregion FSTREAM

#pragma region STREAM
		/**
		 * @function to_stream(std::ostream&, const Keyword&, const std::string&, const unsigned = 3u)
		 * @brief Insert a Keyword into an output stream in human-readable format.
		 * @param os				- Target output stream.
		 * @param kywd				- Target Keyword.
		 * @param indentation		- String to use as indentation before each line.
		 * @param repeatIndentation	- Repeats the indentation string this many times before the effect name.
		 * @returns std::ostream&
		 */
		std::ostream& to_stream(std::ostream& os, const Keyword& kywd, const std::string& indentation, const unsigned repeatIndentation = 3u) const
		{
			for ( auto i{ 0u }; i < repeatIndentation; ++i )
				os << indentation;
			os << color::f::gray << kywd._name << '\n';
			return os;
		}
		/**
		 * @function to_stream(std::ostream&, const Effect&, const std::string&, const std::string&)
		 * @brief Insert an Effect into an output stream in human-readable format.
		 * @param os				- Target output stream.
		 * @param fx				- Target Effect.
		 * @param search_str		- Search string, used to highlight searched-for strings in the output.
		 * @param indentation		- String to use as indentation before each line.
		 * @param repeatIndentation	- Repeats the indentation string this many times before the effect name.
		 * @param ind_fac			- Subtract the number of used chars from this value to get final indentation when printing magnitude & duration.
		 * @returns std::ostream&
		 */
		std::ostream& to_stream(std::ostream& os, const Effect& fx, const std::string& search_str, const std::string& indentation, const unsigned repeatIndentation = 2u, const size_t ind_fac = 25u) const
		{
			const auto [pre, highlight, post]{ get_tuple(fx._name, search_str) };
			const auto fx_color{ resolveEffectColor(fx) };
			for ( auto i{ 0u }; i < repeatIndentation; ++i )
				os << indentation;
			sys::colorSet(fx_color);
			os << pre << color::reset;
			sys::colorSet(_color_highlight);
			os << highlight;
			sys::colorSet(fx_color);
			os << post << color::reset;
			const auto insert_num{ [&os, &ind_fac](const std::string& num, const short color, const unsigned indent) -> unsigned {  // NOLINT(clang-diagnostic-c++20-extensions)
				if (indent > ind_fac)
					os << std::setw(indent + 2u) << ' ';
				else
					os << std::setw(ind_fac - indent) << ' ';
				sys::colorSet(color);
				os << num;
				return num.size();
			} };
			auto size_factor{ fx._name.size() };
			if ( fx._magnitude > 0.0 || _all )
				size_factor = insert_num(str::to_string(fx._magnitude, _precision), _color_fx_mag, size_factor) + 10u;
			if ( fx._duration > 0u || _all ) {
				insert_num(str::to_string(fx._duration, _precision), _color_fx_dur, size_factor);
				os << 's';
			}
			os << color::reset << '\n';
			if ( _verbose || _all )
				for ( auto& KYWD : fx._keywords )
					to_stream(os, KYWD, indentation);
			return os;
		}
		/**
		 * @function to_stream(std::ostream&, const Effect&, const std::string&, const std::string&)
		 * @brief Insert an Effect into an output stream in human-readable format.
		 * @param os				- Target output stream.
		 * @param fx				- Target Effect.
		 * @param search_strings	- Search strings, used to highlight searched-for strings in the output.
		 * @param indentation		- String to use as indentation before each line.
		 * @param repeatIndentation	- Repeats the indentation string this many times before the effect name.
		 * @param ind_fac			- Subtract the number of used chars from this value to get final indentation when printing magnitude & duration.
		 * @returns std::ostream&
		 */
		std::ostream& to_stream(std::ostream& os, const Effect& fx, const std::vector<std::string>& search_strings, const std::string& indentation, const unsigned repeatIndentation = 2u, const size_t ind_fac = 25u) const
		{
			const auto [pre, highlight, post]{ get_tuple(fx._name, search_strings) };
			const auto fx_color{ resolveEffectColor(fx) };
			for ( auto i{ 0u }; i < repeatIndentation; ++i )
				os << indentation;
			sys::colorSet(fx_color);
			os << pre << color::reset;
			sys::colorSet(_color_highlight);
			os << highlight;
			sys::colorSet(fx_color);
			os << post << color::reset;
			const auto insert_num{ [&os, &ind_fac](const std::string& num, const short color, const unsigned indent) -> unsigned {  // NOLINT(clang-diagnostic-c++20-extensions)
				if (indent > ind_fac)
					os << std::setw(indent + 2u) << ' ';
				else
					os << std::setw(ind_fac - indent) << ' ';
				sys::colorSet(color);
				os << num;
				return num.size();
			} };
			auto size_factor{ fx._name.size() };
			if ( fx._magnitude > 0.0 || _all )
				size_factor = insert_num(str::to_string(fx._magnitude, _precision), _color_fx_mag, size_factor) + 10u;
			if ( fx._duration > 0u || _all ) {
				insert_num(str::to_string(fx._duration, _precision), _color_fx_dur, size_factor);
				os << 's';
			}
			os << color::reset << '\n';
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
		std::ostream& to_stream(std::ostream& os, const Ingredient& ingr, const std::string& search_str = "") const
		{
			const auto indentation{ std::string(_indent, ' ') }; // get indentation string
			const auto [pre, highlight, post]{ get_tuple(ingr._name, search_str) };
			os << indentation; // insert indentation
			sys::colorSet(_color); // set color
			os << pre << color::reset;
			sys::colorSet(_color_highlight);
			os << highlight << color::reset;
			sys::colorSet(_color); // set color
			os << post << color::reset << '\n';
			for ( auto& fx : get_fx(ingr._effects, { search_str }) ) // iterate through this ingredient's effects, and insert them as well.
				to_stream(os, fx, search_str, indentation, 2u, 25u);
			return os;
		}

		/**
		 * @function to_fstream(std::ostream&, const SortedIngrList&, const std::vector<std::string>&) const
		 * @brief Insert a list of ingredients into an output stream. Used for the list output mode.
		 * @param os				- Target output stream.
		 * @param ingr				- Target ingredient list.
		 * @param search_strings	- Search strings, used to highlight searched-for strings in the output.
		 * @returns std::ostream&
		 */
		std::ostream& to_stream(std::ostream& os, const SortedIngrList& ingr, const std::vector<std::string>& search_strings) const
		{
			const auto indentation{ std::string(_indent, ' ') };
			const auto to_stream{ [this, &os, &search_strings, &indentation](const SortedIngrList::iterator it) {
				os << indentation;
				sys::colorSet(_color);
				os << it->_name << color::reset << '\n';
				for ( auto& fx : it->_effects )
					this->to_stream(os, fx, search_strings, indentation);
			} };
			if ( _reverse_output )
				for ( auto it{ ingr.rbegin() }; it != ingr.rend(); ++it )
					to_stream(it.base());
			else
				for ( auto it{ ingr.begin() }; it != ingr.end(); ++it )
					to_stream(it);
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
			os << pre << color::reset;
			sys::colorSet(_color_highlight);
			os << highlight << color::reset;
			sys::colorSet(_color); // set color
			os << post << color::reset << '\n';
			for ( auto& fx : potion.effects() ) // iterate through this ingredient's effects, and insert them as well.
				to_stream(os, fx, "", indentation, 2u, 25u);
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
		std::ostream& to_stream_build(std::ostream& os, const Ingredient& ingr, const Potion& potion) const
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
			os << ingr._name << color::reset << '\n';
			for ( auto& fx : get_fx(ingr._effects, names_lc) ) // iterate through this ingredient's effects, and insert them as well.
				to_stream(os, fx, "", indentation);
			return os;
		}

		std::ostream& list_to_stream(std::ostream& os, const SortedIngrList& list) const
		{
			const auto indentation{ std::string(_indent, ' ') };
			for ( auto& it : list )
				to_stream(os, it);
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

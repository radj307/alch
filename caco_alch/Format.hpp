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
		bool _quiet, _verbose, _exact, _all, _file_export, _reverse_output; // see UserAssist.hpp
		size_t _indent, _precision;
		unsigned short _color;

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
		 * @param indent			- How many space characters to include before ingredient names. This is multiplied by 2 for effect names.
		 * @param precision			- How many decimal points of precision to use when outputting floating points.
		 * @param color				- General color override, changes the color of Ingredient names for search, list, and build.
		 */
		explicit Format(const bool quiet = false, const bool verbose = true, const bool exact = false, const bool all = false, const bool file_export = false, const bool reverse_output = false, const size_t indent = 3, const size_t precision = 2u, const unsigned short color = Color::_f_white) : _quiet{ quiet }, _verbose{ verbose }, _exact{ exact }, _all{ all }, _file_export{ file_export }, _reverse_output{ reverse_output }, _indent{ indent }, _precision{ precision }, _color{ color } {}

		[[nodiscard]] bool quiet() const { return _quiet; }
		[[nodiscard]] bool verbose() const { return _verbose; }
		[[nodiscard]] bool exact() const { return _exact; }
		[[nodiscard]] bool all() const { return _all; }
		[[nodiscard]] bool file_export() const { return _file_export; }
		[[nodiscard]] bool reverse_output() const { return _reverse_output; }
		[[nodiscard]] size_t indent() const { return _indent; }
		[[nodiscard]] size_t precision() const { return _precision; }
		[[nodiscard]] unsigned short color() const { return _color; }

		[[nodiscard]] std::tuple<std::string, std::string, std::string> get_tuple(const std::string& str, const std::string& name_lowercase) const
		{
			if ( !str.empty() )
				if ( const auto dPos{ str::tolower(str).find(name_lowercase) }; dPos != std::string::npos )
					return { str.substr(0, dPos), str.substr(dPos, name_lowercase.size()), str.substr(dPos + name_lowercase.size()) };
			return{ str, { }, { } };
		}

		std::vector<Effect*> get_fx(std::array<Effect, 4>& arr, const std::string& name_lowercase) const
		{
			std::vector<Effect*> vec;
			for ( auto it{ arr.begin() }; it != arr.end(); ++it ) {
				if ( !_quiet )
					vec.push_back(&*it);
				else if ( const auto lc{ str::tolower(it->_name) }; lc == name_lowercase || ( !_exact && lc.find(name_lowercase) != std::string::npos ) ) {
					vec.push_back(&*it);
					if ( _exact ) break;
				}
			}
			return vec;
		}

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

		std::ostream& to_stream(std::ostream& os, const Effect& fx, const std::string& search_str, const std::string& indentation) const
		{
			const auto fx_tuple{ get_tuple(fx._name, search_str) };
			os << indentation << indentation << std::get<0>(fx_tuple) << Color::reset << Color::f_yellow << std::get<1>(fx_tuple) << Color::reset << std::get<2>(fx_tuple);
			if ( fx._magnitude > 0.0 || _all ) {
				if ( fx._name.size() > 30u ) // name is long
					os << std::setw(fx._name.size()) << ' ';
				else // name is normal length
					os << std::setw(30 - fx._name.size()) << ' ';
				os << Color::f_gray << fx._magnitude;
				if ( _verbose || _all )
					os << '\t' << fx._duration << 's';
				os << Color::reset << '\n';
			}
			else os << '\n';
			return os;
		}
		std::ostream& to_stream(std::ostream& os, Effect* fx, const std::string& search_str, const std::string& indentation) const
		{
			const auto fx_tuple{ get_tuple(fx->_name, search_str) };
			os << indentation << indentation << std::get<0>(fx_tuple) << Color::reset << Color::f_yellow << std::get<1>(fx_tuple) << Color::reset << std::get<2>(fx_tuple);
			if ( fx->_magnitude > 0.0 || _all ) {
				if ( fx->_name.size() > 30u ) // name is long
					os << std::setw(fx->_name.size()) << ' ';
				else // name is normal length
					os << std::setw(30 - fx->_name.size()) << ' ';
				os << Color::f_gray << fx->_magnitude;
				if ( _verbose || _all )
					os << '\t' << fx->_duration << 's';
				os << Color::reset << '\n';
			}
			else os << '\n';
			return os;
		}
		std::ostream& to_stream(std::ostream& os, Ingredient& ingr, const std::string& search_str) const
		{
			const auto indentation{ std::string(_indent, ' ') }; // get indentation string
			const auto name_tuple{ get_tuple(ingr._name, search_str) };
			os << indentation;
			sys::colorSet(_color);
			os << std::get<0>(name_tuple) << Color::reset << Color::f_yellow << std::get<1>(name_tuple) << Color::reset;
			sys::colorSet(_color);
			os << std::get<2>(name_tuple) << Color::reset << '\n';
			const auto effects{ get_fx(ingr._effects, search_str) };
			for ( auto& fx : effects )
				to_stream(os, fx, search_str, indentation);
			return os;
		}
		std::ostream& to_stream(std::ostream& os, const Effect& fx, const std::string& indentation) const
		{
			os << indentation << indentation << fx._name << std::setw(30 - fx._name.size()) << ' ' << Color::f_gray;
			if ( fx._magnitude > 0.0 || _all )
				os << fx._magnitude;
			else os << '\t';
			if ( fx._duration > 0 || _all )
				os << '\t' << fx._duration << 's';
			os << Color::reset << '\n';
			return os;
		}
		std::ostream& to_stream(std::ostream& os, const Ingredient& ingr) const
		{
			const auto indentation{ std::string(_indent, ' ') }; // get indentation
			os << indentation;
			if ( _color != Color::_f_white )
				sys::colorSet(_color);
			else sys::colorSet(Color::_f_yellow);
			os << ingr._name << Color::reset << '\n';
			if ( !_quiet && !_all )
				for ( auto& fx : ingr._effects )
					to_stream(os, fx, indentation);
			return os;
		}

		Format& operator()() { return *this; }
	};
}
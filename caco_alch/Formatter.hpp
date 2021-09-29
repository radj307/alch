#pragma once
#include <Params.hpp>
#include <INI.hpp>
#include <ColorConfigLoader.hpp>
#include <OutputFormat.hpp>
#include <Potion.hpp>

namespace caco_alch {

	struct Formatter : OutputFormat {
		explicit Formatter(const opt::Params& args, const unsigned indent = 2u) : OutputFormat(
			args.check_flag('q'),
			args.check_flag('v'),
			args.check_flag('e'),
			args.check_flag('a'),
			args.check_flag('E'),
			args.check_flag('R'),
			args.check_flag('c'),
			args.check_flag('S'),
			indent,
			[&args]() { const auto val{ args.getv("precision") }; return val.has_value() && !val.value().empty() ? str::stoui(val.value()) : 2u; }(),
			std::move(buildColorConfigFrom(DefaultObjects._default_colors))
		) {}
		explicit Formatter(const file::ini::INI& ini, const opt::Params& args, const unsigned indent = 2u) : OutputFormat(
			args.check_flag('q'),
			args.check_flag('v'),
			args.check_flag('e'),
			args.check_flag('a'),
			args.check_flag('E'),
			args.check_flag('R'),
			args.check_flag('c'),
			args.check_flag('S'),
			indent,
			[&args]() { const auto val{ args.getv("precision") }; return val.has_value() && !val.value().empty() ? str::stoui(val.value()) : 2u; }(),
			std::move(loadColorConfig(ini))
		) {}

		/**
		 * @brief Split a given name by searched subwords.
		 * @param name				- The ingredient/effect name to split.
		 * @param search_strings	- The searched-for words to highlight.
		 * @returns std::tuple<std::string, std::string, std::string>
		 *\n		0	- This contains the characters that appear before the searched for string if found, otherwise contains the whole name.
		 *\n		1	- This contains the search string if found, otherwise is empty.
		 *\n		2	- This contains the characters that appear after the searched for string if found, otherwise is empty.
		 */
		[[nodiscard]] std::tuple<std::string, std::string, std::string> split_name(const std::string& name, const std::vector<std::string>& search_strings) const
		{
			if (!name.empty())
				for (auto& search_str : search_strings)
					if (const auto pos{ str::tolower(name).find(str::tolower(search_str)) }; str::pos_valid(pos))
						return { name.substr(0u, pos), name.substr(pos, search_str.size()), name.substr(pos + search_str.size()) };
			return{ name, {}, {} };
		}
		/**
		 * @brief Split a given name by a single searched subword.
		 * @param name			- The ingredient/effect name to split.
		 * @param search_str	- The searched-for word to highlight.
		 * @returns std::tuple<std::string, std::string, std::string>
		 *\n		0	- This contains the characters that appear before the searched for string if found, otherwise contains the whole name.
		 *\n		1	- This contains the search string if found, otherwise is empty.
		 *\n		2	- This contains the characters that appear after the searched for string if found, otherwise is empty.
		 */
		[[nodiscard]] std::tuple<std::string, std::string, std::string> split_name(const std::string& name, const std::string& search_str) const
		{
			return split_name(name, { search_str });
		}
		[[nodiscard]] std::tuple<std::string, std::string, std::string> split_name(const std::string& name, const std::optional<std::vector<std::string>>& search_strings) const
		{
			return search_strings.has_value() ? split_name(name, search_strings.value()) : std::make_tuple(name, std::string{}, std::string{});
		}
		/**
		 * @brief Convert an Ingredient's Effect array into a vector of Effects, and applies the _flag_quiet logic to omit any non-matching effect names.
		 * @param effects			- An Ingredient's Effect array.
		 * @param search_strings	- Searched-for subwords.
		 * @returns std::vector<Effect>
		 */
		[[nodiscard]] std::vector<Effect> vectorize_effects(const std::array<Effect, 4>& effects, const std::vector<std::string>& search_strings) const
		{
			std::vector<Effect> vec;
			vec.reserve(effects.size());
			for (auto fx{ effects.begin() }; fx != effects.end(); ++fx) {
				if (!_flag_quiet)
					vec.emplace_back(*fx);
				else if (const auto name{ str::tolower(fx->_name) }; std::any_of(search_strings.begin(), search_strings.end(), [this, &name](const std::string& search_str) {
					return match(name, search_str);
					}))
					vec.emplace_back(*fx);
			}
			vec.shrink_to_fit();
			return vec;
		}
		[[nodiscard]] std::vector<Effect> vectorize_effects(const std::array<Effect, 4>& effects) const
		{
			std::vector<Effect> vec;
			vec.reserve(effects.size());
			for (auto& fx : effects)
				vec.emplace_back(fx);
			vec.shrink_to_fit();
			return vec;
		}

		/**
		 * @brief Get a ColorSetter instance associated with the given effect's keywords, or if _flag_color is false, the default effect color.
		 * @param effect	- An Effect instance.
		 * @returns ColorAPI::ColorSetter
		 */
		[[nodiscard]] ColorAPI::ColorSetter getEffectColorizer(const Effect& effect) const
		{
			if (_flag_color) {
				if (!effect._keywords.empty()) {
					if (hasNegative(effect))
						return _colors.set(UIElement::EFFECT_NAME_NEGATIVE);
					if (hasPositive(effect))
						return _colors.set(UIElement::EFFECT_NAME_POSITIVE);
					if (!effect.hasKeyword(Keywords::KYWD_MagicInfluence))
						return _colors.set(UIElement::EFFECT_NAME_NEUTRAL);
				}
				// use fallback instead. (possible return values: 0,1,2)
				switch (hasKeywordTypeFallback(str::tolower(effect._name))) {
				case 0: // neutral
					return _colors.set(UIElement::EFFECT_NAME_NEUTRAL);
				case 1: // negative
					return _colors.set(UIElement::EFFECT_NAME_NEGATIVE);
				case 2: // positive
					return _colors.set(UIElement::EFFECT_NAME_POSITIVE);
				default:break;
				}
			}
			return _colors.set(UIElement::EFFECT_NAME_DEFAULT); // colors are disabled
		}

		/**
		 * @struct Indentation
		 * @brief Used to correctly increment & insert indentation strings in output streams.
		 */
		struct Indentation {
			char ch;
			size_t rep;
			std::string indent;

			Indentation() : ch{ ' ' }, rep{ 0u }, indent{ std::string(rep, ch) } {}
			Indentation(size_t rep, char ch = '\t') : ch{ std::move(ch) }, rep{ std::move(rep) }, indent{ std::string(rep, ch) } {}

			Indentation getNext(const size_t increase_rep_by = 1u) const
			{
				return{ rep + increase_rep_by, ch };
			}

			Indentation operator()() const { return getNext(); }

			friend std::ostream& operator<<(std::ostream& os, const Indentation& obj)
			{
				os << obj.indent;
				return os;
			}
		};

		/**
		 * @brief Used by the print() function to return an object capable of using std::ostream operator<<.
		 * @tparam T	- Type to print.
		 */
		template<class T> struct ToStream {
			Formatter* fmt;										///< @brief Pointer to a Formatter instance
			T* obj;												///< @brief Pointer to the object to print
			Indentation indent;									///< @brief Indentation object
			std::optional<std::vector<std::string>> searched;	///< @brief Optional searchlist used to highlight searched terms.
			std::streamsize suffix_indent_width;				///< @brief The stream width to use as a maximum limit
			ColorAPI* colors;									///< @brief Pointer to a ColorAPI instance

			/**
			 * @brief Default Constructor
			 * @param format			- Formatter instance
			 * @param object			- Object to print
			 * @param indentation		- Indentation instance
			 * @param search_strings	- Optional searched terms
			 * @param suffixIndent		- Optional max indent for duration values when following a magnitude value.
			 */
			ToStream(Formatter& format, T& object, Indentation indentation, std::optional<std::vector<std::string>> search_strings = std::nullopt, std::streamsize suffixIndent = 25) : fmt{ &format }, obj{ &object }, indent{ std::move(indentation) }, searched{ std::move(search_strings) }, suffix_indent_width{ std::move(suffixIndent) }, colors{ &format._colors } {}

			struct PrintSplitName {
				std::string pre, highlight, post;
				const ColorAPI::ColorSetter& color, hlColor;
				PrintSplitName(std::tuple<std::string, std::string, std::string> splitName, const ColorAPI::ColorSetter& color, const ColorAPI::ColorSetter& highlight_color) : pre{ std::move(std::get<0>(splitName)) }, highlight{ std::move(std::get<1>(splitName)) }, post{ std::move(std::get<2>(splitName)) }, color{ color }, hlColor{ highlight_color } {}

				friend std::ostream& operator<<(std::ostream& os, const PrintSplitName& obj)
				{
					os << obj.color << obj.pre << color::reset << obj.hlColor << obj.highlight << color::reset << obj.color << obj.post << color::reset;
					return os;
				}
			};

			PrintSplitName split_name(const std::string& name, const UIElement& colorT) const
			{
				return PrintSplitName{ fmt->split_name(name, searched), colors->set(colorT), colors->set(UIElement::SEARCH_HIGHLIGHT) };
			}
			PrintSplitName split_name(const std::string& name, const ColorAPI::ColorSetter& colorT) const
			{
				return PrintSplitName{ fmt->split_name(name, searched), colorT, colors->set(UIElement::SEARCH_HIGHLIGHT) };
			}

			// KEYWORD OUTPUT
			friend std::enable_if_t<std::is_same_v<T, Keyword>, std::ostream&> operator<<(std::ostream& os, const ToStream& s)
			{
				if (s.fmt->_flag_export)
					os << "\t\t" << s.obj << '\n';
				else
					os << s.indent << colors->set(UIElement::KEYWORD) << obj->_name << color::reset << '\n';
				return os;
			}
			// EFFECT OUTPUT
			friend std::enable_if_t<std::is_same_v<T, Effect>, std::ostream&> operator<<(std::ostream& os, const ToStream& s)
			{
				if (s.fmt->_flag_export)
					os << '\t' << s.obj._name << "\n\t{\n\t\tmagnitude = " << s.obj._magnitude << "\n\t\tduration = " << s.obj._duration << '\n' << s.obj._keywords << "\t}\n";
				else {
					const Effect& fx{ s.obj };
					os << split_name(fx._name, fmt->getEffectColorizer(fx));

					// print split effect name
					const auto insert_num{ [&os, &s](const std::string& num, const ColorAPI::ColorSetter color, const std::streamsize indent) {
						if (indent > s.suffix_indent_width)
							os << std::setw(s.suffix_indent_width + 2u);
						else
							os << std::setw(s.suffix_indent_width - indent);
						os << ' ' << color << num;
						return num.size();
					} };

					auto size_factor{ s.obj._name.size() };

					if (fx._magnitude > 0.0 || s.fmt->_flag_all)
						size_factor = insert_num(str::to_string(fx._magnitude, s.fmt->_precision), colors->set(UIElement::EFFECT_MAGNITUDE), size_factor) + 10u;
					if (fx._duration > 0u || s.fmt->_flag_all) {
						insert_num(str::to_string(fx._duration, s.fmt->_precision), colors->set(UIElement::EFFECT_DURATION), size_factor);
						os << 's';
					}
					os << color::reset << '\n';
					if (s.fmt->_flag_verbose || s.fmt->_flag_all)
						for (auto& KYWD : fx._keywords)
							os << '\n' << ToStream(*s.fmt, KYWD, s.indent.getNext(), s.searched, s.suffix_indent_width);
				}
				return os;
			}
			// EFFECT VECTOR OUTPUT
			friend std::enable_if_t<std::is_same_v<T, std::vector<Effect>>, std::ostream&> operator<<(std::ostream& os, const ToStream& s)
			{
				for (auto& fx : s.obj)
					os << ToStream(*s.fmt, fx, s.indent, s.searched, s.suffix_indent_width) << '\n';
				return os;
			}
			// INGREDIENT OUTPUT
			friend std::enable_if_t<std::is_same_v<T, Ingredient>, std::ostream&> operator<<(std::ostream& os, const ToStream& s)
			{
				if (s.fmt->_flag_export) {
					os << s.obj._name << "\n{\n";
					for (auto& fx : s.obj._effects)
						os << ToStream(*s.fmt, fx, s.indent.getNext(), s.searched, s.suffix_indent_width) << '\n';
				}
				else {
					os << split_name(s.obj._name, UIElement::INGREDIENT_NAME) << '\n' << ToStream(*s.fmt, fmt->vectorize_effects(s.obj._effects), s.indent.getNext());
				}
				return os;
			}
			// INGREDIENT LIST OUTPUT
			friend std::enable_if_t<std::is_same_v<T, IngrList>, std::ostream&> operator<<(std::ostream& os, const ToStream& s)
			{
				for (auto& it : s.obj)
					os << ToStream(*s.fmt, it, s.indent.getNext(), s.searched, s.suffix_indent_width) << '\n';
				return os;
			}
			// SORTED INGREDIENT LIST OUTPUT
			friend std::enable_if_t<std::is_same_v<T, SortedIngrList>, std::ostream&> operator<<(std::ostream& os, const ToStream& s)
			{
				for (auto& it : s.obj)
					os << ToStream(*s.fmt, it, s.indent.getNext(), s.searched, s.suffix_indent_width) << '\n';
				return os;
			}
			// POTION OUTPUT
			friend std::enable_if_t<std::is_same_v<T, Potion>, std::ostream&> operator<<(std::ostream& os, const ToStream& s)
			{
				if (s.fmt->_flag_export) {
					os << s.obj->name() << "\n{\n" << ToStream(*s.fmt, fmt->vectorize_effects(s.obj->effects()), s.indent.getNext());
				}
				else {
					os << s.colors->set(UIElement::POTION_NAME) << s.obj->name() << color::reset << '\n' << ToStream(*s.fmt, fmt->vectorize_effects(s.obj->effects()), s.indent.getNext());
				}
				return os;
			}
		};

		template<class T>
		ToStream<T> print(T&& obj, Indentation&& ind, std::optional<std::vector<std::string>>&& searched = std::nullopt) const
		{
			return{ *this, std::forward<T>(obj), std::forward<Indentation>(ind), std::forward<std::optional<std::vector<std::string>>>(searched) };
		}
		template<class T>
		ToStream<T> print(T&& obj, std::optional<std::vector<std::string>>&& searched = std::nullopt) const
		{
			return{ *this, std::forward<T>(obj), Indentation{ indent() }, std::forward<decltype(searched)>(searched)};
		}
	};
}
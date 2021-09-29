#pragma once
#include <set>
#include <iomanip>
#include <strconv.hpp>

#include <using.h>
#include <ColorAPI.hpp>
#include <Potion.hpp>
#include <Ingredient.hpp>
#include <ColorConfigLoader.hpp>
#include <OutputFormat.hpp>

namespace caco_alch {
	/**
	 * @struct Format
	 * @brief Provides formatting information for some output stream methods in the Alchemy class.
	 */
	struct Format : OutputFormat {
		Format(auto&& args, auto&& ini) : OutputFormat(std::forward<decltype(args)>(args), std::forward<decltype(ini)>(ini)) {}

		/**
		 * @function split_name(const std::string&, const std::string&) const
		 * @brief Uses a string to split of a line into a tuple of strings where { <preceeding text>, <delimiter text>, <trailing text> }
		 * @param str			The line to split.
		 * @param highlight		Substr of str to separate. Must be converted to lowercase beforehand or match will not be successful.
		 * @returns std::tuple<std::string, std::string, std::string>
		 *\n		0	- Text that preceeds highlight
		 *\n		1	- highlight
		 *\n		2	- Text that supersedes highlight
		 */
		[[nodiscard]] std::tuple<std::string, std::string, std::string> split_name(const std::string& str, const std::string& highlight) const
		{
			if (!str.empty())
				if (const auto dPos{ str::tolower(str).find(highlight) }; dPos != std::string::npos)
					return { str.substr(0, dPos), str.substr(dPos, highlight.size()), str.substr(dPos + highlight.size()) };
			return{ str, { }, { } }; // by default, return blanks for unused tuple slots. This is used to hide the text color when nothing was found.
		}
		/**
		 * @function split_name(const std::string&, const std::string&) const
		 * @brief Uses a string to split of a line into a tuple of strings where { <preceeding text>, <delimiter text>, <trailing text> }
		 * @param str			The line to split.
		 * @param highlights	Substrings of str to separate. Must be converted to lowercase beforehand or match will not be successful.
		 * @returns std::tuple<std::string, std::string, std::string>
		 *\n		0	- Text that preceeds highlight
		 *\n		1	- highlight
		 *\n		2	- Text that supersedes highlight
		 */
		[[nodiscard]] std::tuple<std::string, std::string, std::string> split_name(const std::string& str, const std::vector<std::string>& highlights) const
		{
			if (!str.empty())
				for (auto& highlight : highlights)
					if (const auto dPos{ str::tolower(str).find(highlight) }; dPos != std::string::npos)
						return { str.substr(0, dPos), str.substr(dPos, highlight.size()), str.substr(dPos + highlight.size()) };
			return{ str, { }, { } }; // by default, return blanks for unused tuple slots. This is used to hide the text color when nothing was found.
		}

		/**
		 * @function vectorize_effects(std::array<Effect, 4>&, const std::vector<std::string>&) const
		 * @brief Retrieve a vector of Effects from an Ingredient Effect array.
		 * @param arr				Effect array ref.
		 * @param names_lowercase	The names to search for, must be lowercase.
		 * @returns std::vector<Effect*>
		 */
		[[nodiscard]] std::vector<Effect> vectorize_effects(const std::array<Effect, 4>& arr, const std::vector<std::string>& names_lowercase) const
		{
			std::vector<Effect> vec;
			vec.reserve(4llu);
			for (auto it{ arr.begin() }; it != arr.end(); ++it) {
				if (!_flag_quiet)
					vec.push_back(*it);
				else if (const auto lc{ str::tolower(it->_name) }; std::any_of(names_lowercase.begin(), names_lowercase.end(), [this, &lc](const std::string& name) -> bool { return match(lc, name); })) {
					vec.push_back(*it);
					if (_flag_exact)
						break;
				}
			}
			return vec;
		}
		/**
		 * @brief Retrieve a color to use when outputting a given effect.
		 * @param effect	- The effect to check.
		 * @returns ColorAPI::ColorSetter
		 */
		ColorAPI::ColorSetter resolveEffectColor(const Effect& effect) const
		{
			// check if color override is enabled
			if (_flag_color)
				return _colors.set(UIElement::EFFECT_NAME_NEUTRAL);
			// check keywords
			if (!effect._keywords.empty()) {
				if (hasNegative(effect))
					return _colors.set(UIElement::EFFECT_NAME_NEGATIVE);
				if (hasPositive(effect))
					return _colors.set(UIElement::EFFECT_NAME_POSITIVE);
				if (!effect.hasKeyword(Keywords::KYWD_MagicInfluence))
					return _colors.set(UIElement::EFFECT_NAME_NEUTRAL);
			}
			return _colors.set(UIElement::EFFECT_NAME_DEFAULT); // else return white
		}

		/**
		 * @struct Indentation
		 * @brief Used to correctly increment & insert indentation strings in output streams.
		 */
		struct Indentation {
			std::streamsize rep, incr;

			Indentation() : rep{ 0llu }, incr{ 0llu } {}
			Indentation(std::streamsize repeat) : rep{ std::move(repeat) }, incr{ rep } {}
			Indentation(std::streamsize repeat, std::streamsize increment) : rep{ std::move(repeat) }, incr{ std::move(increment) } {}

			Indentation getNext() const
			{
				return{ rep + incr, incr };
			}

			Indentation operator()() const { return getNext(); }

			friend std::ostream& operator<<(std::ostream& os, const Indentation& obj)
			{
				for (std::streamsize i{ 0llu }; i < obj.rep; ++i)
					os << ' ';
				return os;
			}
		};
		struct PrintableBase {
			using SearchedType = std::optional<std::vector<std::string>>;
		protected:
			const Format& fmt;
			Indentation ind;
			SearchedType searched;
			PrintableBase(const Format& format, Indentation indent, SearchedType searched) : fmt{ format }, ind{ std::move(indent) }, searched{ std::move(searched) } {}
			virtual ~PrintableBase() = default;

			virtual std::vector<Effect> getEffects(const Ingredient& ingr) const
			{
				if (searched.has_value())
					return fmt.vectorize_effects(ingr._effects, searched.value());
				std::vector<Effect> vec;
				vec.reserve(4);
				for (auto& fx : ingr._effects)
					vec.emplace_back(fx);
				vec.shrink_to_fit();
				return vec;
			}
		public:
			PrintableBase() = delete;
		};
		/**
		 * @struct PrintObject
		 * @brief caco_alch object printer functor.
		 */
		struct PrintObject : PrintableBase {
		private:
			/**
			 * @brief Print an object reference to the given output stream.
			 * @param os	- Output Stream Ref
			 * @param kywd	- Keyword
			 */
			void print(std::ostream& os, const Keyword& kywd) const
			{
				if (fmt.file_export())
					os << ind << kywd._form_id << " = " << kywd._name;
				else {
					os << ind << fmt._colors.set(UIElement::KEYWORD) << kywd._name << color::reset;
					if (fmt.all())
						os << std::setw(30llu - kywd._name.size()) << fmt._colors.set(UIElement::KEYWORD) << kywd._form_id << color::reset;
				}
			}
			/**
			 * @brief Print an object reference to the given output stream.
			 * @param os	- Output Stream Ref
			 * @param kwda	- KeywordList
			 */
			void print(std::ostream& os, const KeywordList& kwda) const
			{
				for (auto kywd{ kwda.begin() }; kywd != kwda.end(); ) {
					os << PrintObject(*kywd, *this);
					if (++kywd != kwda.end())
						os << '\n';
				}
			}
			/**
			 * @brief Print an object reference to the given output stream.
			 * @param os	- Output Stream Ref
			 * @param mgef	- Effect
			 */
			void print(std::ostream& os, const Effect& mgef) const
			{
				if (fmt.file_export()) {
					const auto nextIndent{ ind.getNext() };
					os << ind << mgef._name << '\n' << nextIndent << "{\n" << PrintObject(mgef, *this, true) << '\n' << nextIndent << "}";
				}
				else {
					const auto insert_num{ [&os](const std::string& num, const ColorAPI::ColorSetter color, const unsigned indent) -> unsigned {  // NOLINT(clang-diagnostic-c++20-extensions)
						if (indent > 25llu)
							os << std::setw(static_cast<std::streamsize>(indent) + 2u) << ' ';
						else
							os << std::setw(25llu - static_cast<std::streamsize>(indent)) << ' ';
						os << color << num;
						return num.size();
					} };
					auto szFactor{ mgef._name.size() };
					os << ind;
					// Print the effect's name
					if (searched.has_value()) { // split effect name
						const auto [pre, highlight, post] {fmt.split_name(mgef._name, searched.value())};
						const auto color{ fmt.resolveEffectColor(mgef) };
						os << color << pre << color::reset << fmt._colors.set(UIElement::SEARCH_HIGHLIGHT) << highlight << color::reset << color << post << color::reset;
					}
					else // don't split effect name if searched is null
						os << fmt.resolveEffectColor(mgef) << mgef._name << color::reset;
					// Print the effect's magnitude & duration
					os << std::setw(30llu - mgef._name.size());
					szFactor = insert_num(str::to_string(mgef._magnitude, fmt._precision), fmt._colors.set(UIElement::EFFECT_MAGNITUDE), szFactor) + 10llu;
					os << color::reset;
					insert_num(str::to_string(mgef._duration, fmt._precision), fmt._colors.set(UIElement::EFFECT_DURATION), szFactor);
					os << 's' << color::reset;
					if ((fmt.verbose() || fmt.all()) && !mgef._keywords.empty())
						os << '\n' << PrintObject(mgef._keywords, *this, true);
				}
			}
			/**
			 * @brief Print an object reference to the given output stream.
			 * @param os	- Output Stream Ref
			 * @param list	- EffectList
			 */
			void print(std::ostream& os, const EffectList& list) const
			{
				for (auto fx{ list.begin() }; fx != list.end(); ++fx) {
					if (!fmt.quiet()) {
						os << PrintObject(*fx, *this);
						if (fx + 1 != list.end())
							os << '\n';
					}
					else if (searched.has_value()) {
						if (const auto lc_name{ str::tolower(fx->_name) }; std::any_of(searched.value().begin(), searched.value().end(), [&lc_name, this](std::string searched) {
							searched = str::tolower(searched);
							return lc_name == searched || !fmt._flag_exact && str::pos_valid(lc_name.find(searched));
							})) {
							os << PrintObject(*fx, *this);
							if (fx + 1 != list.end())
								os << '\n';
						}
					}
				}
			}
			/**
			 * @brief Print an object reference to the given output stream.
			 * @param os	- Output Stream Ref
			 * @param ingr	- Ingredient
			 */
			void print(std::ostream& os, const Ingredient& ingr) const
			{
				if (fmt.file_export()) {
					os << ind << ingr._name << "\n{\n";
					os << PrintObject(getEffects(ingr), *this, true);
					os << "\n}";
				}
				else {
					// print ingredient name
					os << ind;
					if (searched.has_value()) {
						const auto [pre, highlight, post] { fmt.split_name(ingr._name, searched.value()) };
						os << fmt._colors.set(UIElement::INGREDIENT_NAME) << pre << color::reset << fmt._colors.set(UIElement::SEARCH_HIGHLIGHT) << highlight << color::reset << fmt._colors.set(UIElement::INGREDIENT_NAME) << post << color::reset;
					}
					else
						os << fmt._colors.set(UIElement::INGREDIENT_NAME) << ingr._name << color::reset;
					if (const auto effects{ getEffects(ingr) }; !effects.empty())
						os << '\n' << PrintObject(effects, *this, true);
				}
			}
			/**
			 * @brief Print an object reference to the given output stream.
			 * @param os	- Output Stream Ref
			 * @param list	- SortedIngrList
			 */
			void print(std::ostream& os, const SortedIngrList& list) const
			{
				for (auto ingr{ list.begin() }; ingr != list.end(); ) {
					os << PrintObject(*ingr, *this);
					if (++ingr != list.end())
						os << '\n';
				}
			}
			/**
			 * @brief Print an object reference to the given output stream.
			 * @param os	 - Output Stream Ref
			 * @param potion - Potion
			 */
			void print(std::ostream& os, const Potion& potion) const
			{
				if (fmt.file_export()) {
					const auto nextIndent{ ind.getNext() };
					os << ind << potion._name << '\n' << ind << "{\n" << PrintObject(potion.effects(), *this, true) << "\n}";
				}
				else
					os << ind << fmt._colors.set(UIElement::POTION_NAME) << potion._name << color::reset << '\n' << PrintObject(potion.effects(), *this, true);
			}

		public:
			using Variant = std::variant<std::monostate, Keyword, KeywordList, Effect, EffectList, Ingredient, SortedIngrList, Potion>;
			Variant obj;	///< @brief Copy of the object to be printed.
			/**
			 * @brief Default Constructor
			 * @param obj		- Object to print, of a valid Variant type.
			 * @param fmt		- Format instance.
			 * @param indent	- Indentation to use.
			 * @param searched	- Optional vector of search strings, used to highlight searched-for words in the output.
			 */
			PrintObject(Variant obj, const Format& fmt, Indentation indent, PrintableBase::SearchedType searched) : PrintableBase(fmt, std::move(indent), std::move(searched)), obj{ std::move(obj) } {}
			/**
			 * @brief Recursion Constructor
			 * @param obj		- Object to print, of a valid Variant type.
			 * @param other		- Another PrintObject instance.
			 * @param getNext	- Whether to call the getNext() function on the other instance's Indentation functor.
			 */
			PrintObject(Variant obj, const PrintObject& other, const bool getNext = false) : PrintableBase(other.fmt, (getNext ? std::move(other.ind.getNext()) : other.ind), other.searched), obj{ std::move(obj) } {}

			/**
			 * @brief Acts as a type selector & api for the various print() functions.
			 * @param os	- (implicit) Output stream.
			 * @param p		- (implicit) PrintObject instance.
			 * @returns std::ostream&
			 */
			friend std::ostream& operator<<(std::ostream& os, const PrintObject& p)
			{
				// Keyword
				if (std::holds_alternative<Keyword>(p.obj))
					p.print(os, std::get<Keyword>(p.obj));
				// Keyword List
				else if (std::holds_alternative<KeywordList>(p.obj))
					p.print(os, std::get<KeywordList>(p.obj));
				// Effect
				else if (std::holds_alternative<Effect>(p.obj))
					p.print(os, std::get<Effect>(p.obj));
				// Effect List
				else if (std::holds_alternative<EffectList>(p.obj))
					p.print(os, std::get<EffectList>(p.obj));
				// Ingredient
				else if (std::holds_alternative<Ingredient>(p.obj))
					p.print(os, std::get<Ingredient>(p.obj));
				// Sorted Ingredient List
				else if (std::holds_alternative<SortedIngrList>(p.obj))
					p.print(os, std::get<SortedIngrList>(p.obj));
				// Potion
				else if (std::holds_alternative<Potion>(p.obj))
					p.print(os, std::get<Potion>(p.obj));
				return os;
			}
		};
		PrintObject print(PrintObject::Variant obj, PrintableBase::SearchedType searched = std::nullopt) const
		{
			return{ std::move(obj), *this, { _indent }, searched };
		}

		/**
		 * @operator()()
		 * @brief Retrieve a reference to this Format instance.
		 * @returns Format&
		 */
		Format& operator()() { return *this; }
	};
}

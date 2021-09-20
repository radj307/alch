#pragma once
#include <Colorlib.hpp>
#include <vector>
#include <optional>
#include <functional>

namespace caco_alch {
	/// TO ADD NEW UI ELEMENTS
	/// 1.	Define a new enumerator in UIElement
	/// 2.	Add an entry to the switch statement in getColorIdentifier with the desired INI key name for the UI element.
	/// 3.	Add an entry in DefaultObjects::_default_colors for the new UI element, define a default color.
	/// Done!

	/**
	 * @brief List of UI Elements
	 */
	enum class UIElement {
		BRACKET,
		SEARCH_HIGHLIGHT,
		SEARCH_HEADER,
		INGREDIENT_NAME,
		POTION_NAME,
		ALCHEMY_SKILL,
		EFFECT_NAME_DEFAULT,
		EFFECT_NAME_POSITIVE,
		EFFECT_NAME_NEGATIVE,
		EFFECT_NAME_NEUTRAL,
		EFFECT_MAGNITUDE,
		EFFECT_DURATION,
		KEYWORD,
	};

	/**
	 * @brief Retrieve the (regular) name of the color key associated with each UI element. Key names listed here may also appear as "<keyname>_bold" to make that color bold.
	 * @param ui	- UI Element to check.
	 * @returns std::string
	 */
	inline std::string getColorIdentifier(const UIElement& ui)
	{
		using enum UIElement;
		switch (ui) {
		case BRACKET:
			return "bracket";
		case SEARCH_HIGHLIGHT:
			return "search";
		case SEARCH_HEADER:
			return "header";
		case INGREDIENT_NAME:
			return "ingredient";
		case POTION_NAME:
			return "potion";
		case ALCHEMY_SKILL:
			return "alchemy_skill";
		case EFFECT_NAME_DEFAULT:
			return "effect_name";
		case EFFECT_NAME_POSITIVE:
			return "effect_positive";
		case EFFECT_NAME_NEGATIVE:
			return "effect_negative";
		case EFFECT_NAME_NEUTRAL:
			return "effect_neutral";
		case EFFECT_MAGNITUDE:
			return "magnitude";
		case EFFECT_DURATION:
			return "duration";
		case KEYWORD:
			return "keyword";
		}
		return{};
	}

	/**
	 * @struct Color
	 * @brief Wrapper for variable TermAPI color that have integrated bold formatting support and an operator<<.
	 */
	struct Color {
		short _color;
		bool _bold;
		bool _foreground;

		constexpr Color(const short color = color::white, const bool bold = false, const bool foreground = true) : _color{ color }, _bold{ false }, _foreground{ foreground } {}

		friend std::ostream& operator<<(std::ostream& os, const Color& obj)
		{
			os << color::setcolor(obj._color, obj._foreground);
			if (obj._bold)
				os << color::bold;
			return os;
		}
	};

	/**
	 * @struct ColorAPI
	 * @brief Contains the color palette, sorted by UI element.
	 */
	struct ColorAPI {
		using PaletteType = std::vector<std::pair<UIElement, Color>>;
		PaletteType _palette;

		explicit ColorAPI() = default;
		explicit ColorAPI(PaletteType color_palette) : _palette{ std::move(color_palette) } {}

		bool isValid() const
		{
			return !_palette.empty();
		}

		PaletteType::const_iterator findUIColor(UIElement ui) const
		{
			for (auto it{ _palette.begin() }; it != _palette.end(); ++it)
				if (ui == it->first)
					return it;
			return _palette.end();
		}

		PaletteType::const_iterator begin() const { return _palette.begin(); }
		PaletteType::const_iterator end() const { return _palette.end(); }

		/**
		 * @brief Functor used to apply UI colors.
		 */
		struct ColorSetter {
			Color _col;

			ColorSetter(const ColorAPI& api, const UIElement& ui)
			{
				if (const auto it{ api.findUIColor(ui) }; it != api.end())
					_col = it->second;
				else _col = Color{};
			}

			friend std::ostream& operator<<(std::ostream& os, const ColorSetter& obj)
			{
				os << obj._col;
				return os;
			}
		};

		/**
		 * @brief Designed to be used inline with output stream expressions, this function sets the console color to the color associated with the given UI Element.
		 * @param ui	- UI Element
		 * @returns ColorSetter
		 */
		ColorSetter set(const UIElement& ui) const
		{
			return{ *this, ui };
		}

		friend std::ostream& operator<<(std::ostream& os, const ColorAPI& obj)
		{
			os << "[color]";
			for (auto& [ui, color] : obj._palette) {
				if (color._bold)
					os << getColorIdentifier(ui) << "_bold = " << color._color << '\n';
				else
					os << getColorIdentifier(ui) << " = " << color._color << '\n';
			}
			return os;
		}
	};
}
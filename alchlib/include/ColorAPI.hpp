#pragma once
#include <TermAPI.hpp>
#include <palette.hpp>
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
	enum class UIElement : char {
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
		switch (ui) {
		case UIElement::BRACKET:
			return "bracket";
		case UIElement::SEARCH_HIGHLIGHT:
			return "search";
		case UIElement::SEARCH_HEADER:
			return "header";
		case UIElement::INGREDIENT_NAME:
			return "ingredient";
		case UIElement::POTION_NAME:
			return "potion";
		case UIElement::ALCHEMY_SKILL:
			return "alchemy_skill";
		case UIElement::EFFECT_NAME_DEFAULT:
			return "effect_name";
		case UIElement::EFFECT_NAME_POSITIVE:
			return "effect_positive";
		case UIElement::EFFECT_NAME_NEGATIVE:
			return "effect_negative";
		case UIElement::EFFECT_NAME_NEUTRAL:
			return "effect_neutral";
		case UIElement::EFFECT_MAGNITUDE:
			return "magnitude";
		case UIElement::EFFECT_DURATION:
			return "duration";
		case UIElement::KEYWORD:
			return "keyword";
		}
		return{};
	}

	/**
	 * @struct Color
	 * @brief Wrapper for variable TermAPI color that have integrated bold formatting support and an operator<<.
	 */
	struct Color : public color::setcolor {
		Color(const short color = color::white, const bool bold = false, const bool foreground = true) : color::setcolor(color, foreground ? color::Layer::F : color::Layer::B)
		{
			if (bold) this->operator+=(color::FormatFlag::Bold);
		}
	};

	using PaletteType = color::palette<UIElement>;

	static PaletteType ColorAPI{ {
		{ UIElement::BRACKET, Color{ color::red, true } },
		{ UIElement::SEARCH_HIGHLIGHT, Color{ color::yellow } },
		{ UIElement::SEARCH_HEADER, Color{ color::intense_white } },
		{ UIElement::INGREDIENT_NAME, Color{ color::white, true } },
		{ UIElement::POTION_NAME, Color{ color::white, true } },
		{ UIElement::ALCHEMY_SKILL, Color{ color::cyan } },
		{ UIElement::EFFECT_NAME_DEFAULT, Color{ color::white } },
		{ UIElement::EFFECT_NAME_POSITIVE, Color{ color::green } },
		{ UIElement::EFFECT_NAME_NEGATIVE, Color{ color::red } },
		{ UIElement::EFFECT_NAME_NEUTRAL, Color{ color::white } },
		{ UIElement::EFFECT_MAGNITUDE, Color{ color::magenta } },
		{ UIElement::EFFECT_DURATION, Color{ color::cyan } },
		{ UIElement::KEYWORD, Color{ color::gray } },
		}
	};
}
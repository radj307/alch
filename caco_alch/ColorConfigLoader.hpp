#pragma once
#include <strconv.hpp>
#include <INI.hpp>
#include <ColorAPI.hpp>
#include <DefaultObjects.hpp>

namespace caco_alch {

	/**
	 * @brief Load the color values from the INI config file.
	 * @param ini		- INI instance containing color data.
	 * @param header	- The INI section name containing the color keys. Key names are determined by the getColorIdentifier() function.
	 * @returns ColorAPI
	 */
	inline ColorAPI loadColorConfig(const file::ini::INI& ini, const std::string& header = "color")
	{
		ColorAPI::PaletteType vec; // make a new palette
		vec.reserve(DefaultObjects._default_colors.size());

		// iterate through the default color palette, choose replacements where possible.
		for (auto& [ui, defColor] : DefaultObjects._default_colors) {
			// regular color
			if (const auto identifier{ getColorIdentifier(ui) }; ini.check(header, identifier))
				vec.emplace_back(std::make_pair(ui, Color{ str::stos(ini.getv(header, identifier)) }));
			// bold color
			else if (const auto bold_identifier{ identifier + "_bold" }; ini.check(header, bold_identifier))
				vec.emplace_back(std::make_pair(ui, Color{ str::stos(ini.getv(header, bold_identifier)), true }));
			// default color
			else
				vec.emplace_back(std::make_pair(ui, defColor));
		}

		vec.shrink_to_fit();
		return ColorAPI(std::move(vec));
	}

	inline bool saveColorConfig(const std::string& filename, const ColorAPI& colors)
	{
		std::stringstream ss;
		ss << colors;
		return file::write(filename, ss, false);
	}

	inline ColorAPI buildColorConfigFrom(const ColorAPI::PaletteType& palette)
	{
		return ColorAPI{ palette };
	}
}
#pragma once
#include "INamedObject.hpp"

#include <sysarch.h>
#include <var.hpp>
#include <make_exception.hpp>
#include <fileio.hpp>

#include <nlohmann/json.hpp>

#include <variant>

namespace alchlib2 {
	using GameSettingValueVariant_t = std::variant<std::monostate, std::string, float, int, unsigned, bool>;

	template<typename T> concept ValidGameSettingValueType =
		var::any_same_or_convertible<T, std::string, float, int, unsigned, bool>;

	enum GameSettingType : std::uint8_t {
		Null,
		String,
		Float,
		Int,
		Bool,
	};

	template<ValidGameSettingValueType T>
	inline GameSettingType GetGameSettingType()
	{
		if constexpr (std::same_as<T, std::string>)
			return GameSettingType::String;
		else if constexpr (std::same_as<T, float>)
			return GameSettingType::Float;
		else if constexpr (std::same_as<T, int> || std::same_as<T, unsigned>)
			return GameSettingType::Int;
		else if constexpr (std::same_as<T, bool>)
			return GameSettingType::Bool;
		return GameSettingType::Null;
	}

	struct GameSettingBase : INamedObject {
		STRCONSTEXPR GameSettingBase(GameSettingType const& type) : type{ type } {}
		STRCONSTEXPR GameSettingBase(const std::string& name, GameSettingType const& type) : INamedObject(name), type{ type }
		{
			if (!validate_gmst_naming_scheme(name)) {
				throw make_exception("Unexpected GMST editor ID '", name, "'; expected a name starting with 'f', 's', 'i', or 'b'!");
			}
		}

		CONSTEXPR GameSettingType GetType() const noexcept { return type; }

	private:
		GameSettingType type;

		STRCONSTEXPR static bool validate_gmst_naming_scheme(const std::string& name)
		{
			if (name.empty()) return false;

			const auto& fst{ name.at(0) };
			//       float    ||   string   ||  integral  ||   boolean
			return fst == 'f' || fst == 's' || fst == 'i' || fst == 'b';
		}
	};

	template<ValidGameSettingValueType T>
	struct GameSetting : GameSettingBase {
		STRCONSTEXPR GameSetting() : GameSettingBase(GetGameSettingType<T>()) {}
		STRCONSTEXPR GameSetting(const std::string& name, const T& value) : GameSettingBase(name, GetGameSettingType<T>()), value{ value } {}

		T value;

		CONSTEXPR operator T& () noexcept { return value; }
		CONSTEXPR operator T() const noexcept { return value; }

		NLOHMANN_DEFINE_TYPE_INTRUSIVE(GameSetting, name, value);
	};

	struct AlchemyCoreGameSettings {
		GameSetting<float> fAlchemyIngredientInitMult{ "fAlchemyIngredientInitMult", 3.0f };
		GameSetting<float> fAlchemySkillFactor{ "fAlchemySkillFactor", 3.0f };
		GameSetting<float> fAlchemyAV{ "fAlchemyAV", 15.0f };
		GameSetting<float> fAlchemyMod{ "fAlchemyMod", 0.0f };

		STRCONSTEXPR AlchemyCoreGameSettings() = default;

		NLOHMANN_DEFINE_TYPE_INTRUSIVE(AlchemyCoreGameSettings,
									   fAlchemyIngredientInitMult,
									   fAlchemySkillFactor,
									   fAlchemyAV,
									   fAlchemyMod);

		[[nodiscard]] static AlchemyCoreGameSettings ReadFrom(const std::filesystem::path& path)
		{
			nlohmann::json j;
			file::read(path) >> j;
			return j.get<AlchemyCoreGameSettings>();
		}
		static bool WriteTo(const std::filesystem::path& path, const AlchemyCoreGameSettings& coreGameSettings)
		{
			std::stringstream ss;
			ss << nlohmann::json{ coreGameSettings };
			return file::write_to(path, std::move(ss), false);
		}
	};
}

#pragma once
#include "GameSetting.hpp"

namespace alchlib2 {
	template<typename TReturn, typename... TArgs>
	struct FormulaBase {
		using this_t = FormulaBase<TReturn, TArgs...>;

		virtual ~FormulaBase() = default;

		virtual TReturn GetResult(TArgs...) const = 0;
	};

	struct AlchemyCoreFormula : FormulaBase<float, float> {
		AlchemyCoreFormula(const AlchemyCoreGameSettings& coreGameSettings) : coreGameSettings{ coreGameSettings } {}

		const AlchemyCoreGameSettings& coreGameSettings;

		float GetResult(const float base_val) const override
		{
			return base_val
				* coreGameSettings.fAlchemyIngredientInitMult
				* (1.0f + coreGameSettings.fAlchemyAV / 200.0f)
				* (1.0f + (coreGameSettings.fAlchemySkillFactor - 1.0f))
				* (coreGameSettings.fAlchemyAV / 100.0f)
				* (1.0f + coreGameSettings.fAlchemyMod / 100.0f);
		}

		/**
		 * @brief					Calculate the actual base value from the given starting base value.
		 * @param base_val			Starting effect stat base value.
		 * @param coreGameSettings	An AlchemyCoreGameSettings instance to use.
		 * @returns					The actual base value after applying the core alchemy formula.
		 */
		static float CalculateBase(const float base_val, const AlchemyCoreGameSettings& coreGameSettings)
		{
			return AlchemyCoreFormula{ coreGameSettings }.GetResult(base_val);
		}
	};
}

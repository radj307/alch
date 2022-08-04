#pragma once
#include "ObjectBase.hpp"

#include <strconv.hpp>

#include <vector>

namespace caco_alch {
	/**
	 * @struct Keyword
	 * @brief Represents a KYWD record.
	 */
	struct Keyword : ObjectBase {
		std::string _form_id;

		Keyword(const std::string& name, const std::string& formID) : ObjectBase(name), _form_id{ formID } {}
		explicit Keyword(const std::string& name) : ObjectBase(name) {}

		bool operator==(const Keyword& o) const { return _name == o._name || _form_id == o._form_id; }
		bool operator==(const std::string& name_or_id) const { return _name == name_or_id || _form_id == name_or_id; }
		bool operator!=(auto&& o) const { return !operator==(std::forward<decltype(o)>(o)); }

		bool operator<(const Keyword& o) const { return _name < o._name || _form_id < o._form_id; }
		bool operator>(const Keyword& o) const { return _name > o._name || _form_id > o._form_id; }

		bool is_similar(const std::string& name_or_id) const
		{
			const auto lc_in{ str::tolower(name_or_id) };
			return operator==(lc_in) || str::pos_valid(str::tolower(_name).find(lc_in)) || str::pos_valid(str::tolower(_form_id).find(lc_in));
		}
		bool is_similar(const Keyword& kywd) const
		{
			const auto lc_inname{ str::tolower(kywd._name) }, lc_infid{ str::tolower(kywd._form_id) }, lc_name{ str::tolower(_name) }, lc_fid{ str::tolower(_form_id) };
			return operator==(lc_inname) || operator==(lc_infid) || str::pos_valid(lc_name.find(lc_inname)) || str::pos_valid(lc_fid.find(lc_infid));
		}

		friend std::ostream& operator<<(std::ostream& os, const Keyword& kywd)
		{
			os << kywd._form_id << " = " << kywd._name;
			return os;
		}
	};
	using KeywordList = std::set<Keyword, _internal::less<Keyword>>; ///< @brief Represents a KWDA record type.

	/**
	 * @namespace Keywords
	 * @brief Contains the list of valid effect keywords (KYWD).
	 */
	namespace Keywords {
		inline const Keyword
			// Potion/Poison
			KYWD_Beneficial{ "MagicAlchBeneficial",				"000F8A4E" }, ///< @brief Keyword used to specify a positive effect.
			KYWD_CACO_CureDisease{ "MagicAlchCureDisease_CACO",		"7E90B902" }, ///< @brief Keyword used to specify a positive effect.
			KYWD_CACO_CurePoison{ "MagicAlchCurePoison_CACO",			"7E90B903" }, ///< @brief Keyword used to specify a positive effect.
			KYWD_Harmful{ "MagicAlchHarmful",					"00042509" }, ///< @brief Keyword used to specify a negative effect.
			KYWD_MagicInfluence{ "MagicInfluence",					"00078098" },
			// Special
			KYWD_DurationBased{ "MagicAlchDurationBased",			"000F8A4F" }, ///< @brief Keyword used to determine whether an ingredient is magnitude-based or duration-based.
			// Health
			KYWD_RestoreHealth{ "MagicAlchRestoreHealth",			"00042503" }, ///< @brief Effect restores health
			KYWD_DamageHealth{ "MagicAlchDamageHealth",			"0010F9DD" }, ///< @brief Effect damages health
			KYWD_FortifyHealth{ "MagicAlchFortifyHealth",			"00065A31" }, ///< @brief Effect fortifies health value.
//			KYWD_FortifyRegenHealth{		"MagicAlchFortifyHealthRate",		"" }, // doesn't exist?
			// Stamina
			KYWD_RestoreStamina{ "MagicAlchRestoreStamina",			"00042504" }, ///< @brief Effect restores stamina.
			KYWD_DamageStamina{ "MagicAlchDamageStamina",			"0010F9DC" }, ///< @brief Effect damages stamina.
			KYWD_FortifyStamina{ "MagicAlchFortifyStamina",			"00065A32" }, ///< @brief Effect fortifies stamina value.
			KYWD_FortifyRegenStamina{ "MagicAlchFortifyStaminaRate",		"00065A35" }, ///< @brief Effect fortifies stamina regeneration.
			KYWD_CACO_Fatigue{ "MagicAlchFatigue_CACO",			"7E07A153" }, ///< @brief Effect fortifies stamina regeneration.
			// Magicka
			KYWD_RestoreMagicka{ "MagicAlchRestoreMagicka",			"00042508" }, ///< @brief Effect restores magicka.
			KYWD_DamageMagicka{ "MagicAlchDamageMagicka",			"0010F9DE" }, ///< @brief Effect damages magicka.
			KYWD_FortifyMagicka{ "MagicAlchFortifyMagicka",			"00065A33" }, ///< @brief Effect fortifies magicka value.
			KYWD_FortifyRegenMagicka{ "MagicAlchFortifyMagickaRate",		"00065A34" }, ///< @brief Effect fortifies magicka regeneration.
			KYWD_CACO_FortifyRegenMagicka{ "MagicAlchDamageMagickaRegen_CACO",	"7E07A152" }, ///< @brief Effect fortifies magicka regeneration.
			KYWD_CACO_Silence{ "MagicAlchSilence_CACO",			"7E07A150" }, ///< @brief Effect fortifies magicka regeneration.
			// Skills
			KYWD_FortifyLockpicking{ "MagicAlchFortifyLockpicking",		"00065A26" },
			KYWD_FortifySpeechcraft{ "MagicAlchFortifySpeechcraft",		"00065A29" };
		inline const KeywordList
			positive{
				KYWD_Beneficial,
				KYWD_CACO_CureDisease,
				KYWD_CACO_CurePoison,
				KYWD_RestoreHealth,
				KYWD_RestoreMagicka,
				KYWD_RestoreStamina,
				KYWD_FortifyHealth,
				KYWD_FortifyMagicka,
				KYWD_FortifyStamina,
				KYWD_CACO_FortifyRegenMagicka,
				KYWD_FortifyRegenMagicka,
				KYWD_FortifyRegenStamina
		},
			negative{
				KYWD_Harmful,
				KYWD_DamageHealth,
				KYWD_DamageMagicka,
				KYWD_DamageStamina,
				KYWD_CACO_Fatigue,
				KYWD_CACO_Silence
		};
	}

	/**
	 * @operator<<(std::ostream&, const KeywordList&)
	 * @brief Stream insertion operator for a list of keywords, uses specific indentation for file output.
	 * @param os	- The target output stream.
	 * @param KWDA	- A KeywordList object.
	 * @returns std::ostream&
	 */
	inline std::ostream& operator<<(std::ostream& os, const KeywordList& KWDA)
	{
		const auto indentation{ std::string(2u, '\t') };
		os << indentation << "Keywords\n" << indentation << "{\n";
		for (auto& KYWD : KWDA)
			os << indentation << '\t' << KYWD._form_id << " = " << KYWD._name << '\n';
		os << indentation << "}\n\t";
		return os;
	}
}
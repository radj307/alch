// ReSharper disable CppClangTidyClangDiagnosticExitTimeDestructors
#pragma once
#include "ObjectBase.hpp"

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
		bool operator!=(const Keyword& o) const { return _name != o._name || _form_id != o._form_id; }

		bool operator<(const Keyword& o) const { return _name < o._name || _form_id < o._form_id; }
		bool operator>(const Keyword& o) const { return _name > o._name || _form_id > o._form_id; }

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
			KYWD_Beneficial{			"MagicAlchBeneficial",			"000F8A4E" }, ///< @brief Keyword used to specify a positive effect.
			KYWD_Harmful{				"MagicAlchHarmful",				"00042509" }, ///< @brief Keyword used to specify a negative effect.
			// Special
			KYWD_DurationBased{			"MagicAlchDurationBased",		"000F8A4F" }, ///< @brief Keyword used to determine whether an ingredient is magnitude-based or duration-based.
			// Health
			KYWD_RestoreHealth{			"MagicAlchRestoreHealth",		"00042503" }, ///< @brief Effect restores health
			KYWD_DamageHealth{			"MagicAlchDamageHealth",		"0010F9DD" }, ///< @brief Effect damages health
			KYWD_FortifyHealth{			"MagicAlchFortifyHealth",		"00065A31" }, ///< @brief Effect fortifies health value.
			KYWD_FortifyRegenHealth{	"MagicAlchFortifyHealthRate",	"" }, // doesn't exist?
			// Stamina
			KYWD_RestoreStamina{		"MagicAlchRestoreStamina",		"00042504" }, ///< @brief Effect restores stamina.
			KYWD_DamageStamina{			"MagicAlchDamageStamina",		"0010F9DC" }, ///< @brief Effect damages stamina.
			KYWD_FortifyStamina{		"MagicAlchFortifyStamina",		"00065A32" }, ///< @brief Effect fortifies stamina value.
			KYWD_FortifyRegenStamina{	"MagicAlchFortifyStaminaRate",	"00065A35" }, ///< @brief Effect fortifies stamina regeneration.
			// Magicka
			KYWD_RestoreMagicka{		"MagicAlchRestoreMagicka",		"00042508" }, ///< @brief Effect restores magicka.
			KYWD_DamageMagicka{			"MagicAlchDamageMagicka",		"0010F9DE" }, ///< @brief Effect damages magicka.
			KYWD_FortifyMagicka{		"MagicAlchFortifyMagicka",		"00065A33" }, ///< @brief Effect fortifies magicka value.
			KYWD_FortifyRegenMagicka{	"MagicAlchFortifyMagickaRate",	"00065A34" }; ///< @brief Effect fortifies magicka regeneration.
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
		for ( auto& KYWD : KWDA )
			os << indentation << '\t' << KYWD._form_id << " = " << KYWD._name << '\n';
		os << indentation << "}\n\t";
		return os;
	}
}
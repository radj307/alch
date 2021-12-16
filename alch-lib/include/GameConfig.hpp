#pragma once
#include "using.h"
#include "ObjectBase.hpp"
#include "Effect.hpp"

#include <str.hpp>
#include <fileio.hpp>

#include <cmath>
#include <utility>

namespace caco_alch {
	namespace _internal {
		// Use internally by the GameConfigBase struct to prevent its type from being changed.
		enum class GMST_T {
			STRING = 0,
			DOUBLE = 1,
			BOOL = 2
		};
		/**
		 * @enum PerkPhysicianType
		 * @brief Contains the list of valid targets for the physician perk.
		 */
		enum class PerkPhysicianType {
			NONE = 0u,
			HEALTH = 1u,
			STAMINA = 2u,
			MAGICKA = 3u,
			ALL = 4u
		};
	}

	/**
	 * @struct GameConfigBase
	 * @brief Represents a single game setting, with one of three types.
	 */
	struct GameConfigBase : ObjectBase {
		using STRING = std::optional<std::string>;	///< @brief Variable type
		using DOUBLE = std::optional<double>;		///< @brief Variable type
		using BOOL = std::optional<bool>;			///< @brief Variable type

		using Tuple = std::tuple<STRING, DOUBLE, BOOL>;
		Tuple _value;
		const _internal::GMST_T _type;

		GameConfigBase(const std::string& name, const std::string& value) : ObjectBase(name), _value{ value, std::nullopt, std::nullopt }, _type{ _internal::GMST_T::STRING } {}
		GameConfigBase(const std::string& name, const double value) : ObjectBase(name), _value{ std::nullopt, value, std::nullopt }, _type{ _internal::GMST_T::DOUBLE } {}
		GameConfigBase(const std::string& name, const bool value) : ObjectBase(name), _value{ std::nullopt, std::nullopt, value }, _type{ _internal::GMST_T::BOOL } {}

		void set(const std::string& value)
		{
			if (_type != _internal::GMST_T::STRING)
				throw std::exception("INVALID_TYPE");
			_value = Tuple{ value, std::nullopt, std::nullopt };
		}
		void set(const double value)
		{
			if (_type != _internal::GMST_T::DOUBLE)
				throw std::exception("INVALID_TYPE");
			_value = Tuple{ std::nullopt, value, std::nullopt };
		}
		void set(const bool value)
		{
			if (_type != _internal::GMST_T::BOOL)
				throw std::exception("INVALID_TYPE");
			_value = Tuple{ std::nullopt, std::nullopt, value };
		}

		/**
		 * @function safe_get() const
		 * @brief Safely retrieve this game setting's value, irrelevant of which type it is.
		 * @returns std::string
		 */
		[[nodiscard]] std::string safe_get() const
		{
			switch (_type) {
			case _internal::GMST_T::BOOL:
				return str::bool_to_string(std::get<2>(_value).value_or(false));
			case _internal::GMST_T::DOUBLE:
				return str::to_string(std::get<1>(_value).value_or(0.0));
			case _internal::GMST_T::STRING:
				return std::get<0>(_value).value_or("");
			}
			return{};
		}

		friend std::ostream& operator<<(std::ostream& os, const GameConfigBase& gmst)
		{
			os << gmst._name << " = ";
			if (const auto str{ std::get<0>(gmst._value) }; str.has_value())
				os << str.value();
			else if (const auto& dbl{ std::get<1>(gmst._value) }; dbl.has_value())
				os << std::to_string(dbl.value());
			else if (const auto b{ std::get<2>(gmst._value) }; b.has_value()) {
				if (b.value())
					os << "true";
				else os << "false";
			}
			return os;
		}
	};

	/**
	 * @struct GameConfig
	 * @brief Contains values parsed from the INI config used in potion building.
	 */
	struct GameConfig {
		using Cont = std::vector<GameConfigBase>;
	private:
		Cont _settings; ///< @brief Game Setting Container.

		/**
		 * @function find(const std::string&)
		 * @brief Retrieve a pointer to the game setting with a given name.
		 * @param name	- Target GameSetting name.
		 * @returns GameConfigBase*
		 */
		GameConfigBase* find(const std::string& name)
		{
			for (auto& it : _settings)
				if (it._name == name)
					return &it;
			return nullptr;
		}
	public:
		GameConfig() = delete;
		/**
		 * @constructor GameConfig(Cont)
		 * @brief Copy-Move Constructor
		 * @param settings	- A settings object to import.
		 */
		explicit GameConfig(Cont settings) : _settings{ std::move(settings) } {}
		/**
		 * @constructor GameConfig(Cont, const std::string&)
		 * @brief Default Constructor, reads values from a given file
		 * @param default_settings	- The default settings used to fill in any missing entries.
		 * @param filename			- The filepath to read settings from.
		 */
		explicit GameConfig(Cont default_settings, const std::string& filename) : _settings{ set(std::move(default_settings), file::read(filename)) } {}

	#pragma region GENERIC_GETTERS_SETTERS
		/**
		 * @function find(const std::string&, const int = 0) const
		 * @brief Retrieve an iterator to a given game setting based on its name.
		 * @returns Cont::const_iterator
		 */
		[[nodiscard]] Cont::const_iterator find(const std::string& name, const int off = 0, const bool& fuzzy = false) const
		{
			return std::find_if(off + _settings.begin(), _settings.end(), [&name, &fuzzy](const GameConfigBase& setting) { return setting._name == name || fuzzy && str::tolower(setting._name) == str::tolower(name); });
		}
		[[nodiscard]] constexpr Cont::const_iterator begin() const
		{
			return _settings.begin();
		}
		[[nodiscard]] constexpr Cont::const_iterator end() const
		{
			return _settings.end();
		}
		/**
		 * @function getValue(const std::string&, const int = 0) const
		 * @brief Wrapper for find() & GameConfigBase::safe_get()
		 * @returns std::string
		 */
		[[nodiscard]] std::string getValue(const std::string& name, const int off = 0) const
		{
			if (const auto it{ find(name, off) }; it != _settings.end())
				return it->safe_get();
			return{};
		}
		/**
		 * @function getBoolValue(const std::string&, const int = 0) const
		 * @brief Retrieve a boolean value from the config file.
		 * @returns bool
		 * @throws std::exception("INVALID_OPERATION")	- Requested value is not a boolean.
		 */
		[[nodiscard]] bool getBoolValue(const std::string& name, const int off = 0) const
		{
			try {
				return std::get<GameConfigBase::BOOL>(find(name, off)->_value).value();
			} catch (std::exception&) {
				throw std::exception(std::string("Failed to retrieve \'" + name + '\'').c_str());
			}
		}
		/**
		 * @function getDoubleValue(const std::string&, const int = 0) const
		 * @brief Retrieve a double value from the config file.
		 * @returns double
		 * @throws std::exception("INVALID_OPERATION")	- Requested value is not a double.
		 */
		[[nodiscard]] double getDoubleValue(const std::string& name, const int off = 0) const
		{
			try {
				return std::get<GameConfigBase::DOUBLE>(find(name, off)->_value).value();
			} catch (std::exception&) {
				throw std::exception(std::string("Failed to retrieve \'" + name + '\'').c_str());
			}
		}
		/**
		 * @function getStringValue(const std::string&, const int = 0) const
		 * @brief Retrieve a string value from the config file.
		 * @returns std::string
		 * @throws std::exception("INVALID_OPERATION")	- Requested value is not a string.
		 */
		[[nodiscard]] std::string getStringValue(const std::string& name, const int off = 0) const
		{
			try {
				return std::get<GameConfigBase::STRING>(find(name, off)->_value).value();
			} catch (std::exception&) {
				throw std::exception(std::string("Failed to retrieve \'" + name + '\'').c_str());
			}
		}

		/**
		 * @brief			Set the value of a specified game setting.
		 * @param target	The target game setting.
		 * @param end		The end position of the container. This is used to check if target exists.
		 * @param value_str	The value to set the given target to, as a string.
		 * @returns			bool
		 */
		[[nodiscard]] static bool set(const Cont::iterator target, const Cont::iterator end, const std::string& value_str)
		{
			if (target != end) {
				if (std::all_of(value_str.begin(), value_str.end(), [](const char c) { return isdigit(c) || c == '-' || c == '.'; }))
					target->set(str::stod(value_str));
				else if (str::tolower(value_str) == "true")
					target->set(true);
				else if (str::tolower(value_str) == "false")
					target->set(false);
				else
					target->set(value_str);
				return true;
			}
			return false;
		}

		/**
		 * @function set(Cont, std::stringstream&&)
		 * @brief Parse a stringstream containing the game settings config, using a given container as default arguments.
		 * @returns Cont
		 */
		[[nodiscard]] static Cont set(Cont settings, std::stringstream&& ss)
		{
			if (ss.fail()) throw std::exception("INVALID_FILE_FORMAT");
			ss << '\n';
			for (std::string ln{}; std::getline(ss, ln, '\n'); ) {
				ln = str::strip_line(ln);
				if (!ln.empty()) {
					if (const auto dPos{ ln.find('=') }; str::pos_valid(dPos)) {
						const auto var{ str::strip_line(ln.substr(0u, dPos), "") };
						const auto val{ str::strip_line(ln.substr(dPos + 1), "") };
						if (set(std::find_if(settings.begin(), settings.end(), [&var](const Cont::value_type& e) { return e._name == var; }), settings.end(), val)) {  // NOLINT(bugprone-branch-clone)
						#ifdef ENABLE_DEBUG
							std::cout << sys::term::debug << "Successfully read INI variable \'" << var << "\' = \'" << val << '\'' << std::endl;
						#endif
						}
						else {
						#ifdef ENABLE_DEBUG
							std::cout << sys::term::debug << "Skipped unknown INI variable \'" << var << "\' = \'" << val << '\'' << std::endl;
						#endif
						}
					}
				}
			}
			return settings;
		}

		/**
		 * @function set(const std::string&, const std::string&)
		 * @brief Sets the value of a game setting.
		 * @param setting	- The target game setting's name.
		 * @param value_str	- The value to set, as a string. This is automatically converted to the correct type.
		 * @returns bool
		 *			true	- Success.
		 *			false	- Failed to deduce setting type, or failed to find game setting by the given name.
		 */
		bool set(const std::string& setting, const std::string& value_str)
		{
			return set(std::find_if(_settings.begin(), _settings.end(), [&setting](const Cont::value_type& e) { return e._name == setting; }), _settings.end(), value_str);
		}

		/**
		 * @function read_ini(const std::string&)
		 * @brief Read an INI file containing valid Game Setting data.
		 * @param filename	- The path to the target INI file.
		 */
		void read_ini(const std::string& filename)
		{
			_settings = set(_settings, file::read(filename));
		}
	#pragma endregion GENERIC_GETTERS_SETTERS

	#pragma region GMST_GETTERS
		[[nodiscard]] double fAlchemyIngredientInitMult() const
		{
			return getDoubleValue("fAlchemyIngredientInitMult");
		}
		[[nodiscard]] double fAlchemySkillFactor() const
		{
			return getDoubleValue("fAlchemySkillFactor");
		}
		[[nodiscard]] unsigned fAlchemyAV() const
		{
			return static_cast<unsigned>(std::round(getDoubleValue("fAlchemyAV")));
		}
		[[nodiscard]] double fAlchemyMod() const
		{
			return getDoubleValue("fAlchemyMod");
		}
		/**
		 * @function fPerkAlchemyMasteryRank() const
		 * @brief Retrieve the player's "Alchemy Mastery" perk rank.
		 * @returns double	- Possible values: ( 0.0 | 1.0 | 2.0 )
		 */
		[[nodiscard]] unsigned fPerkAlchemyMasteryRank() const
		{
			auto val{ getDoubleValue("fPerkAlchemyMasteryRank") };
			if (val > 2.0)
				val = 2.0;
			else if (val < 0.0)
				val = 0.0;
			return static_cast<unsigned>(std::round(val));
		}
		[[nodiscard]] bool bPerkPoisoner() const
		{
			return getBoolValue("bPerkPoisoner");
		}
		[[nodiscard]] bool bPerkAdvancedLab() const
		{
			return getBoolValue("bPerkAdvancedLab");
		}
		[[nodiscard]] bool bPerkThatWhichDoesNotKillYou() const
		{
			return getBoolValue("bPerkThatWhichDoesNotKillYou");
		}
		[[nodiscard]] bool bPerkBenefactor() const
		{
			return getBoolValue("bPerkBenefactor");
		}
		[[nodiscard]] double fPerkPoisonerFactor() const
		{
			return getDoubleValue("fPerkPoisonerFactor");
		}
		[[nodiscard]] _internal::PerkPhysicianType sPerkPhysicianType() const
		{
			using namespace _internal;
			const auto str{ str::tolower(getStringValue("sPerkPhysicianType")) };
			if (str::pos_valid(str.find("health")))
				return PerkPhysicianType::HEALTH;
			if (str::pos_valid(str.find("stamina")))
				return PerkPhysicianType::STAMINA;
			if (str::pos_valid(str.find("magicka")))
				return PerkPhysicianType::MAGICKA;
			if (str::pos_valid(str.find("beneficial")) || str::pos_valid(str.find("all")))
				return PerkPhysicianType::ALL;
			return PerkPhysicianType::NONE;
		}
		/**
		 * @function bPerkPhysicianAppliesTo(const Effect&) const
		 * @brief Check if the physician perk applies to a given Effect.
		 * @param effect	- The effect to check.
		 */
		[[nodiscard]] bool bPerkPhysicianAppliesTo(const Effect& effect) const
		{
			using namespace _internal;
			using namespace Keywords;
			switch (sPerkPhysicianType()) {
			case PerkPhysicianType::HEALTH:
				return effect.hasKeyword(KYWD_RestoreHealth, KYWD_FortifyHealth);
			case PerkPhysicianType::STAMINA:
				return effect.hasKeyword(KYWD_RestoreStamina, KYWD_FortifyStamina, KYWD_FortifyRegenStamina);
			case PerkPhysicianType::MAGICKA:
				return effect.hasKeyword(KYWD_RestoreMagicka, KYWD_FortifyMagicka, KYWD_FortifyRegenMagicka, KYWD_CACO_FortifyRegenMagicka);
			case PerkPhysicianType::ALL:
				return effect.hasKeyword(KYWD_Beneficial) || effect.hasKeyword(positive);
			case PerkPhysicianType::NONE:
				return false;
			}
			return false;
		}
		[[nodiscard]] bool bPerkPureMixture() const
		{
			return getBoolValue("bPerkPureMixture");
		}
	#pragma endregion GMST_GETTERS
	private:
		[[nodiscard]] double calculate_base(const double base_val, const unsigned avAlchemy) const
		{
			const auto AVAlchemy{ static_cast<double>(avAlchemy) };
			return	base_val
				* fAlchemyIngredientInitMult()
				* (1.0 + AVAlchemy / 200.0)
				* (1.0 + (fAlchemySkillFactor() - 1.0))
				* (AVAlchemy / 100.0)
				* (1.0 + fAlchemyMod() / 100.0);
		}
		[[nodiscard]] double calculate_perks(double val, const Effect& effect, const unsigned avAlchemy) const
		{
			const auto AVAlchemy{ static_cast<double>(avAlchemy) };
			const auto perk_mastery{ fPerkAlchemyMasteryRank() };
			if (perk_mastery == 1u)
				val *= 1.2;
			else if (perk_mastery == 2u)
				val *= 1.4;
			if (bPerkAdvancedLab())
				val *= 1.25;
			if (bPerkBenefactor() && effect.hasKeyword(Keywords::KYWD_Beneficial))
				val *= 1.25;
			if (bPerkPoisoner() && effect.hasKeyword(Keywords::KYWD_Harmful) && !effect.hasKeyword(Keywords::KYWD_Beneficial))
				val *= 1.0 + AVAlchemy * fPerkPoisonerFactor();
			if (bPerkThatWhichDoesNotKillYou())
				val *= 1.25;
			return val;
		}

	public:
		/**
		 * @function apply_pure_mixture_perk(EffectList&, const bool)
		 * @brief Applies the Pure Mixture perk to an effect list. (Removes negative effects from positive potions, or positive effects from negative potions.)
		 * @param effects		- Ref of an EffectList instance.
		 * @param rm_positive	- When true, removes positive effects, else removes negative effects.
		 * @returns EffectList&
		 */
		[[nodiscard]] EffectList& apply_pure_mixture_perk(EffectList& effects, const bool rm_positive) const
		{
			if (bPerkPureMixture()) {
				EffectList keep{};
				keep.reserve(effects.size());
				for (auto& it : effects)
					if ((!rm_positive && it.hasKeyword(Keywords::positive)) || (rm_positive && it.hasKeyword(Keywords::negative)))
						keep.push_back(it);
				keep.shrink_to_fit();
				effects = keep;
				effects.shrink_to_fit();
			}
			return effects;
		}

		/**
		 * @function calculate(const double) const
		 * @brief Calculate the magnitude of an effect using its magnitude and the current game settings.
		 * @param effect	- Target effect.
		 * @returns double
		 */
		[[nodiscard]] Effect calculate(const Effect& effect) const
		{
			const auto AlchemyAV{ fAlchemyAV() };
			if (effect.hasKeyword(Keywords::KYWD_DurationBased)) {
				double mag{ std::round(calculate_perks(calculate_base(effect._magnitude, AlchemyAV), effect, AlchemyAV)) };
				return Effect{ effect._name, mag, effect._duration, effect._keywords };
			}
			unsigned dur{ static_cast<unsigned>(std::round(calculate_perks(calculate_base(effect._duration, AlchemyAV), effect, AlchemyAV))) };
			return Effect{ effect._name, effect._magnitude, dur, effect._keywords };
		}

		/**
		 * @function get_power_factor() const
		 * @brief Calculate the power factor used in potion brewing using the current game settings.
		 * @returns double
		 */
		[[nodiscard]] double get_power_factor() const
		{
			return fAlchemyIngredientInitMult() * (1.0 + (fAlchemySkillFactor() - 1.0) * fAlchemyAV() / 100.0);
		}

		/**
		 * @operator<<(std::ostream&, const GameConfig&)
		 * @brief Stream insertion operator.
		 * @param os	- Target output stream.
		 * @param gs	- Target GameConfig instance.
		 * @returns std::ostream&
		 */
		friend std::ostream& operator<<(std::ostream& os, const GameConfig& gs)
		{
			for (auto& it : gs._settings)
				os << it << '\n';
			os.flush();
			return os;
		}

		/**
		 * @function to_stream() const
		 * @brief Converts gamesettings into a stringstream using operator<<.
		 * @returns std::stringstream
		 */
		[[nodiscard]] std::stringstream to_stream() const
		{
			std::stringstream ss;
			ss << *this;
			return ss;
		}
	};
}
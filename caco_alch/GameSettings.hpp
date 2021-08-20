#pragma once
#include <cmath>
#include <utility>
#include <file.h>

namespace caco_alch {
	namespace _internal {
		// Use internally by the GMST struct to prevent its type from being changed.
		enum class GMST_T {
			STRING = 0,
			DOUBLE = 1,
			BOOL = 2
		};
	}

	/**
	 * @struct GMST
	 * @brief Represents a single game setting, with one of three types.
	 */
	struct GMST : ObjectBase {
		using STRING = std::optional<std::string>;	///< @brief Variable type
		using DOUBLE = std::optional<double>;		///< @brief Variable type
		using BOOL = std::optional<bool>;			///< @brief Variable type

		using Tuple = std::tuple<STRING, DOUBLE, BOOL>;
		Tuple _value;
		const _internal::GMST_T _type;

		GMST(const std::string& name, const std::string& value) : ObjectBase(name), _value{ value, std::nullopt, std::nullopt }, _type{ _internal::GMST_T::STRING } {}
		GMST(const std::string& name, const double value) : ObjectBase(name), _value{ std::nullopt, value, std::nullopt }, _type{ _internal::GMST_T::DOUBLE } {}
		GMST(const std::string& name, const bool value) : ObjectBase(name), _value{ std::nullopt, std::nullopt, value }, _type{ _internal::GMST_T::BOOL } {}

		void set(const std::string& value)
		{
			if ( _type != _internal::GMST_T::STRING )
				throw std::exception("INVALID_TYPE");
			_value = Tuple{ value, std::nullopt, std::nullopt };
		}
		void set(const double value)
		{
			if ( _type != _internal::GMST_T::DOUBLE )
				throw std::exception("INVALID_TYPE");
			_value = Tuple{ std::nullopt, value, std::nullopt };
		}
		void set(const bool value)
		{
			if ( _type != _internal::GMST_T::BOOL )
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
				return str::to_string(std::get<2>(_value).value_or(false));
			case _internal::GMST_T::DOUBLE:
				return str::to_string(std::get<1>(_value).value_or(0.0));
			case _internal::GMST_T::STRING:
				return std::get<0>(_value).value_or("");
			}
			return{};
		}

		friend std::ostream& operator<<(std::ostream& os, const GMST& gmst)
		{
			os << gmst._name << " = ";
			if (const auto str{ std::get<0>(gmst._value) }; str.has_value() )
				os << str.value();
			else if (const auto dbl{ std::get<1>(gmst._value) }; dbl.has_value() )
				os << std::to_string(dbl.value());
			else if (const auto b{ std::get<2>(gmst._value) }; b.has_value() ) {
				if ( b.value() )
					os << "true";
				else os << "false";
			}
			return os;
		}
	};

	/**
	 * @struct GameSettings
	 * @brief Contains values parsed from the INI config used in potion building.
	 */
	struct GameSettings {
		using Cont = std::vector<GMST>;
	private:
		Cont _settings; ///< @brief Game Setting Container.

		/**
		 * @function find(const std::string&)
		 * @brief Retrieve a pointer to the game settings with a given name.
		 * @param name	- Target GameSetting name.
		 * @returns GMST*
		 */
		GMST* find(const std::string& name)
		{
			for ( auto& it : _settings )
				if ( it._name == name )
					return &it;
			return nullptr;
		}
	public:
		GameSettings() = delete;
		/**
		 * @constructor GameSettings(Cont)
		 * @brief Copy-Move Constructor
		 * @param settings	- A settings object to import.
		 */
		explicit GameSettings(Cont settings) : _settings{ std::move(settings) } {}
		/**
		 * @constructor GameSettings(Cont, const std::string&)
		 * @brief Default Constructor, reads values from a given file
		 * @param default_settings	- The default settings used to fill in any missing entries.
		 * @param filename			- The filepath to read settings from.
		 */
		explicit GameSettings(Cont default_settings, const std::string& filename) : _settings{ set(std::move(default_settings), file::read(filename)) } {}

#pragma region GENERIC_GETTERS_SETTERS
		/**
		 * @function find(const std::string&, const int = 0) const
		 * @brief Retrieve an iterator to a given game setting based on its name.
		 * @returns Cont::const_iterator
		 */
		[[nodiscard]] Cont::const_iterator find(const std::string& name, const int off = 0) const
		{
			return std::find_if(off + _settings.begin(), _settings.end(), [&name](const GMST& setting) { return setting._name == name; });
		}
		/**
		 * @function getValue(const std::string&, const int = 0) const
		 * @brief Wrapper for find() & GMST::safe_get()
		 * @returns std::string
		 */
		[[nodiscard]] std::string getValue(const std::string& name, const int off = 0) const
		{
			if ( const auto it{ find(name, off) }; it != _settings.end() )
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
				return std::get<GMST::BOOL>(find(name, off)->_value).value();
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
				return std::get<GMST::DOUBLE>(find(name, off)->_value).value();
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
				return std::get<GMST::STRING>(find(name, off)->_value).value();
			} catch (std::exception&) {
				throw std::exception(std::string("Failed to retrieve \'" + name + '\'').c_str());
			}
		}

		[[nodiscard]] static bool set(const Cont::iterator target, const Cont::iterator end, const std::string& value_str)
		{
			if (target != end) {
				try { // catch std::stod exceptions to detect type
					target->set(std::stod(value_str));
				} catch (...) {
					if (str::tolower(value_str) == "true")
						target->set(true);
					else if (str::tolower(value_str) == "false")
						target->set(false);
					else target->set(value_str);
				}
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
			for ( std::string ln{ }; std::getline(ss, ln, '\n'); ) {
				ln = str::strip_line(ln);
				if ( !ln.empty() ) {
					if ( const auto dPos{ ln.find('=') }; str::pos_valid(dPos) ) {
						const auto var{ str::strip_line(ln.substr(0u, dPos), "") };
						const auto val{ str::strip_line(ln.substr(dPos + 1), "") };
						if ( set(std::find_if(settings.begin(), settings.end(), [&var](const Cont::value_type& e) { return e._name == var; }), settings.end(), val) ) {  // NOLINT(bugprone-branch-clone)
						#ifdef ENABLE_DEBUG
							std::cout << sys::debug << "Successfully read INI variable \'" << var << "\' = \'" << val << '\'' << std::endl;
						#endif
						}
						else {
						#ifdef ENABLE_DEBUG
							std::cout << sys::debug << "Skipped unknown INI variable \'" << var << "\' = \'" << val << '\'' << std::endl;
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
		[[nodiscard]] double fAlchemyAV() const
		{
			return getDoubleValue("fAlchemyAV");
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
		[[nodiscard]] double fPerkAlchemyMasteryRank() const
		{
			auto val{ getDoubleValue("fPerkAlchemyMasteryRank") };
			if (val > 2.0)
				val = 2.0;
			else if ( val < 0.0 )
				val = 0.0;
			return std::round(val);
		}
		[[nodiscard]] bool bPerkPoisoner() const
		{
			return getBoolValue("bPerkPoisoner");
		}
#pragma endregion GMST_GETTERS

		/**
		 * @function calculate(const double) const
		 * @brief Calculate the magnitude of an effect using its magnitude and the current game settings.
		 * @param effect	- Target effect.
		 * @returns double
		 */
		[[nodiscard]] Effect calculate(const Effect& effect) const
		{
			const auto AlchemyAV{ fAlchemyAV() };
			const auto base_formula{ [this, &AlchemyAV](const double base_value) -> double {
				return std::round(
					base_value
					* fAlchemyIngredientInitMult()
					* ( 1.0 + AlchemyAV / 200.0 )
					* ( 1.0 + ( fAlchemySkillFactor() - 1.0 )
					* ( AlchemyAV / 100.0 ) )
					* ( 1.0 + fAlchemyMod() / 100.0 )
				);
			} };
			const auto perk_formula{ [this, &AlchemyAV, &effect](double val) -> double {
				const auto perk_mastery{ fPerkAlchemyMasteryRank() };
				if (perk_mastery == 1.0)
					val *= 1.2;
				else if (perk_mastery == 2.0)
					val *= 1.4;
				if (bPerkPoisoner() && effect.hasKeyword(Keywords::KYWD_Harmful))
					val *= 0.5 * AlchemyAV;
				return std::round(val);
			} };
			if (effect.hasKeyword(Keywords::KYWD_DurationBased))
				return Effect{ effect._name, perk_formula(base_formula(effect._magnitude)), effect._duration, effect._keywords };
			return Effect{ effect._name, base_formula(effect._magnitude), static_cast<unsigned>(perk_formula(base_formula(effect._duration))), effect._keywords };
		}

		/**
		 * @function get_power_factor() const
		 * @brief Calculate the power factor used in potion brewing using the current game settings.
		 * @returns double
		 */
		[[nodiscard]] double get_power_factor() const
		{
			return fAlchemyIngredientInitMult() * ( 1.0 + ( fAlchemySkillFactor() - 1.0 ) * fAlchemyAV() / 100.0 );
		}

		/**
		 * @operator<<(std::ostream&, const GameSettings&)
		 * @brief Stream insertion operator.
		 * @param os	- Target output stream.
		 * @param gs	- Target GameSettings instance.
		 * @returns std::ostream&
		 */
		friend std::ostream& operator<<(std::ostream& os, const GameSettings& gs)
		{
			for ( auto& it : gs._settings )
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
#pragma once
#include <cmath>

namespace caco_alch {
	struct GameSettingValue {
	private:
		std::string _name;
		double _value;

	public:
		GameSettingValue(const std::string& name, const double value) : _name{ name }, _value{ value } {}

		std::string name() const { return _name; }
		double value() const { return _value; }

		void set(const double value) { _value = value; }
		void set(const std::string& value) { _value = str::stod(value); }

		friend std::ostream& operator<<(std::ostream& os, const GameSettingValue& gsv)
		{
			os << gsv._name << " = " << gsv._value;
			return os;
		}
	};

	struct GameSettings {
	private:
		using Cont = std::vector<GameSettingValue>;
		Cont _settings{ { "fAlchemyIngredientInitMult", 3.0 }, { "fAlchemySkillFactor", 3.0 }, { "AlchemyAV", 15.0 }, { "AlchemyMod", 0.0 } };

		GameSettingValue* get_ptr(const std::string& name)
		{
			for ( auto& it : _settings )
				if ( it.name() == name )
					return &it;
			return nullptr;
		}
	public:
		double getv(const std::string& name) const
		{
			for ( auto& it : _settings )
				if ( it.name() == name )
					return it.value();
			return -0.0;
		}
		double fAlchemyIngredientInitMult() const { return getv("fAlchemyIngredientInitMult"); }
		double fAlchemySkillFactor() const { return getv("fAlchemySkillFactor"); }
		double AlchemyAV() const { return getv("AlchemyAV"); }
		double AlchemyMod() const { return getv("AlchemyMod"); }

		bool set_fAlchemyIngredientInitMult(const double value) { auto target{ get_ptr("fAlchemyIngredientInitMult") }; if ( target != nullptr ) { target->set(value); return true; } return false; }
		bool set_fAlchemySkillFactor(const double value) { auto target{ get_ptr("fAlchemySkillFactor") }; if ( target != nullptr ) { target->set(value); return true; } return false; }
		bool set_AlchemyAV(const double value) { auto target{ get_ptr("AlchemyAV") }; if ( target != nullptr ) { target->set(value); return true; } return false; }
		bool set_AlchemyMod(const double value) { auto target{ get_ptr("AlchemyMod") }; if ( target != nullptr ) { target->set(value); return true; } return false; }
		void set(std::stringstream&& ss)
		{
			ss << '\n';
			for ( std::string ln{ }; std::getline(ss, ln, '\n'); ) {
				ln = str::strip_line(ln);
				if ( !ln.empty() ) {
					if ( const auto dPos{ ln.find('=') }; str::pos_valid(dPos) ) {
						const auto var{ str::strip_line(ln.substr(0u, dPos), "") };
						const auto val{ str::stod(str::strip_line(ln.substr(dPos + 1), "")) };
						if ( const auto target{ std::find_if(_settings.begin(), _settings.end(), [&var](Cont::value_type& e) { return e.name() == var; }) }; target != _settings.end() && val >= 0.0 ) {
							target->set(val);
							#ifdef ENABLE_DEBUG
							std::cout << sys::debug << "Successfully read INI variable \'" << var << "\' = \'" << val << '\'' << std::endl;
							#endif
						}
					}
				}
			}
		}

		double calculate_magnitude(const double base_mag)
		{
			return std::round(base_mag * fAlchemyIngredientInitMult() * ( 1.0 + ( AlchemyAV() / 200.0 ) ) * ( 1.0 + ( fAlchemySkillFactor() - 1.0 ) * ( AlchemyAV() / 100.0 ) ) * ( 1.0 + ( AlchemyMod() / 100.0 ) ) );
		}
		double get_power_factor()
		{
			return fAlchemyIngredientInitMult() * ( 1.0 + ( fAlchemySkillFactor() - 1.0 ) * AlchemyAV() / 100.0 );
		}

		GameSettings() = default;
		GameSettings(const Cont& settings) {  }
		GameSettings(Cont&& settings) {  }

		friend std::ostream& operator<<(std::ostream& os, const GameSettings& gs)
		{
			for ( auto& it : gs._settings )
				os << it << '\n';
			os.flush();
			return os;
		}

		std::stringstream operator()() const
		{
			std::stringstream ss;
			ss << *this;
			return std::move(ss);
		}
	};
}
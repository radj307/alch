#pragma once
#include "Potion.hpp"

#include <make_exception.hpp>

#include <nlohmann/json.hpp>

namespace alchlib2 {
	class PerkBase : public INamedObject {
	protected:
		STRCONSTEXPR PerkBase(std::string const& name, const bool enable = false) : INamedObject(name), enable{ enable } {}

	public:
		bool enable;

		virtual ~PerkBase() = default;

		/**
		 * @brief			Applies this perk's transformations (if any) to the given Effect.
		 * @param effect	An Effect reference to modify.
		 */
		virtual void ApplyToEffect(Effect&) const noexcept {}

		/**
		 * @brief			Applies this perk's transformations (if any) to the given Potion.
		 * @param potion	A Potion reference to modify.
		 */
		virtual void ApplyToPotion(Potion&) const noexcept {}
	};

	class Perk final {
		void throwIfBaseIsNull() const
		{
			if (base.get() == nullptr) throw make_exception("Perk doesn't have a valid base pointer!");
		}

	public:
		std::unique_ptr<PerkBase> base;

		Perk() = default;

		template<std::derived_from<PerkBase> T>
		Perk(const T& base) : base{ std::make_unique<T>(base) } {}

		template<std::derived_from<PerkBase> T>
		Perk(T* base) : base{ base } {}

		/**
		 * @brief			Applies this perk's transformations (if any) to the given Effect.
		 * @param effect	An Effect reference to modify.
		 */
		void ApplyToEffect(Effect& effect) const
		{
			throwIfBaseIsNull();
			if (!base.get()->enable) return;
			const auto& perk{ *base.get() };
			perk.ApplyToEffect(effect);
		}

		/**
		 * @brief			Applies this perk's transformations (if any) to the given Potion.
		 * @param potion	A Potion reference to modify.
		 */
		void ApplyToPotion(Potion& potion) const
		{
			throwIfBaseIsNull();
			if (!base.get()->enable) return;
			const auto& perk{ *base.get() };
			perk.ApplyToPotion(potion);
		}
	};
}

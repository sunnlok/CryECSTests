#pragma once
#include <atomic>
#include "CrySerialization/Decorators/Range.h"


namespace Components::Damage {


	struct MaxHealth {
		float val = 1;

		void Serialize(Serialization::IArchive& ar)
		{
			ar(val, "maxHealth", "Max Health");
			val = std::max(val, 0.f);
		}
	};

	struct Health {
		float val = 1;

		void Serialize(Serialization::IArchive& ar) 
		{
			ar(val, "health", "Health");
		}
	};

	struct AtomicHealth {

		std::atomic<int> health;
	};

	struct Damage {
		float val = 1;

		void Serialize(Serialization::IArchive& ar)
		{
			ar(val, "damage", "Damage");
		}
	};

	struct TickedDamage {
		float tickTotal = 1;
		float tickRemaining = 1;


		void Serialize(Serialization::IArchive& ar)
		{
			ar(tickTotal, "tickDelay", "Tick Delay");
		}
	};

	struct DoTickDamage {

	};

	struct DoDamage {

	};

	struct Dead {

	};

	struct RemoveIfDead {

	};
}
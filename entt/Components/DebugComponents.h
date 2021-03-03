#pragma once

namespace Components::Debug
{
	struct DebugTransform{};

	struct Sphere {
		float radius = 1.f;
	};

	struct Cone {
		float radius = 1.f;
		float length = 1.f;
	};

	struct Direction {
		Vec3 val = { 0,1.f,0 };
	};

	struct AsArrow {

	};



	struct Color {
		ColorF color1 = Col_Green;
		ColorF color2 = Col_Green;
	};
}
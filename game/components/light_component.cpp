#include "light_component.h"

namespace components {
	light::light() {
		attenuations[0].base_value = 1.f;
		attenuations[1].base_value = 0.00002f;
		attenuations[2].base_value = 0.00007f;
		attenuations[3].base_value = 2000.f;

		wall_attenuations[0].base_value = 1.f;
		wall_attenuations[1].base_value = 0.00002f;
		wall_attenuations[2].base_value = 0.00037f;
		wall_attenuations[3].base_value = 2000.f;
	}
}

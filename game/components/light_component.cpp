#include "light_component.h"

namespace components {
	light::light() {
		constant.base_value = 1.f;
		linear.base_value = 0.00002f;
		quadratic.base_value = 0.00007f;
		max_distance.base_value = 2000.f;

		wall_constant.base_value = 1.f;
		wall_linear.base_value = 0.00002f;
		wall_quadratic.base_value = 0.00037f;
		wall_max_distance.base_value = 2000.f;
	}
}

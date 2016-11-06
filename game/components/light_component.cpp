#include "light_component.h"

namespace components {
	light::light() {
		constant.base_value = 0.4f;
		linear.base_value = 0.00002f;
		quadratic.base_value = 0.00007f;
		max_distance.base_value = 2000.f;

		wall_constant.base_value = 0.4f;
		wall_linear.base_value = 0.00002f;
		wall_quadratic.base_value = 0.000037f;
		wall_max_distance.base_value = 2000.f;

		/*
		
		constant.base_value = 0.7f;
		linear.base_value = 0.00002f - 0.00001f;
		quadratic.base_value = 0.00007f - 0.00005f;
		max_distance.base_value = 2000.f;

		wall_constant.base_value = 0.7f;
		wall_linear.base_value = 0.00002f - 0.00001f;
		wall_quadratic.base_value = 0.000037f - 0.00005f;
		wall_max_distance.base_value = 2000.f;
		*/
	}
}

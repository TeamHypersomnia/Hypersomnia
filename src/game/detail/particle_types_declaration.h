#pragma once
#include "augs/templates/transform_types.h"

struct general_particle;
struct animated_particle;
struct homing_animated_particle;

template <template <class...> class List = type_list>
struct list_of_particle_types {
	using type = List<
		general_particle,
		animated_particle,
		homing_animated_particle
	>;
};

template <template<class...> class List = type_list>
using list_of_particle_types_t = typename list_of_particle_types<List>::type;

template<class F>
void for_each_particle_type(F f) {
	for_each_through_std_get(list_of_particle_types_t<>(), f);
}

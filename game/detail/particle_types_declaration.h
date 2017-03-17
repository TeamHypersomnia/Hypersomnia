#pragma once
#include "augs/templates/list_of.h"

struct general_particle;
struct animated_particle;
struct homing_animated_particle;

template<template<typename> class Mod>
struct tuple_of_particle_types {
	typedef tuple_of_t<Mod,
		general_particle,
		animated_particle,
		homing_animated_particle
	> type;
};

template<template<typename> class Mod>
using tuple_of_particle_types_t = typename tuple_of_particle_types<Mod>::type;

template<class F>
void for_each_particle_type(F f) {
	for_each_in_tuple(tuple_of_particle_types_t<empty_mod>(), f);
}

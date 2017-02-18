#pragma once
#include "augs/templates/tuple_of.h"

struct general_particle;
struct animated_particle;

template<template<typename> class Mod>
struct put_all_particle_types_into {
	typedef tuple_of_t<Mod,
		general_particle,
		animated_particle
	> type;
};

template<template<typename> class Mod>
using put_all_particle_types_into_t = typename put_all_particle_types_into<Mod>::type;

template<class F>
void for_each_particle_type(F f) {
	for_each_in_tuple(put_all_particle_types_into_t<empty_mod>(), f);
}

#pragma once
#include "augs/templates/transform_types.h"

#include "game/detail/perks/haste_perk.h"
#include "game/detail/perks/electric_shield_perk.h"

template<template <typename...> class List>
struct put_all_perks_into {
	using type = List<
		haste_perk,
		electric_shield_perk
	>;
};

template <template <typename...> class List>
using put_all_perks_into_t = typename put_all_perks_into<List>::type;

template <class T>
struct make_perk_instance {
	using type = typename T::instance;
};

template <template <typename...> class List>
using put_all_perk_instances_into_t = transform_types_in_list_t<
	put_all_perks_into_t<List>,
	make_perk_instance
>;

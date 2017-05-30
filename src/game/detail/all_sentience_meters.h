#pragma once
#include <string>
#include "game/assets/game_image_id.h"
#include "augs/misc/value_meter.h"
#include "augs/graphics/pixel.h"

struct sentience_meter_appearance {
	// GEN INTROSPECTOR struct sentience_meter_appearance
	assets::game_image_id icon;
	rgba bar_color;
	std::wstring description;
	// END GEN INTROSPECTOR
};

struct health_meter_instance : value_meter {
	// GEN INTROSPECTOR struct health_meter_instance
	// INTROSPECT BASE value_meter
	// END GEN INTROSPECTOR
};

struct personal_electricity_meter_instance : value_meter {
	// GEN INTROSPECTOR struct personal_electricity_meter_instance
	// INTROSPECT BASE value_meter
	// END GEN INTROSPECTOR
};

struct consciousness_meter_instance : value_meter {
	// GEN INTROSPECTOR struct consciousness_meter_instance
	// INTROSPECT BASE value_meter
	// END GEN INTROSPECTOR
};

struct health_meter {
	using instance = health_meter_instance;
	// GEN INTROSPECTOR struct health_meter
	sentience_meter_appearance appearance;
	// END GEN INTROSPECTOR
};

struct personal_electricity_meter {
	using instance = personal_electricity_meter_instance;
	// GEN INTROSPECTOR struct personal_electricity_meter
	sentience_meter_appearance appearance;
	// END GEN INTROSPECTOR
};

struct consciousness_meter {
	using instance = consciousness_meter_instance;
	// GEN INTROSPECTOR struct consciousness_meter
	sentience_meter_appearance appearance;
	// END GEN INTROSPECTOR
};

template<template <typename...> class List>
struct put_all_meters_into {
	using type = List<
		health_meter,
		personal_electricity_meter,
		consciousness_meter
	>;
};

template <template <typename...> class List>
using put_all_meters_into_t = typename put_all_meters_into<List>::type;

template <class T>
struct make_meter_instance {
	using type = typename T::instance;
};

template <template <typename...> class List>
using put_all_meter_instances_into_t = transform_types_in_list_t<
	put_all_meters_into_t<List>,
	make_meter_instance
>;

using meter_instance_tuple = put_all_meter_instances_into_t<augs::trivially_copyable_tuple>;
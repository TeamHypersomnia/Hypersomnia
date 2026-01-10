#pragma once
#include <cstddef>
#include "augs/misc/profiler_mixin.h"

struct cosmic_profiler : public augs::profiler_mixin<cosmic_profiler> {
	cosmic_profiler();

	// GEN INTROSPECTOR struct cosmic_profiler
	augs::time_measurements reinferring_all_entities = 1;

	augs::amount_measurements<std::size_t> visibility_raycasts = 1;
	augs::amount_measurements<std::size_t> pathfinding_raycasts = 1;
	augs::amount_measurements<std::size_t> total_step_raycasts = 1;

	augs::amount_measurements<std::size_t> entropy_length = 1;

	augs::time_measurements logic;
	augs::time_measurements missiles;
	augs::time_measurements explosives;
	augs::time_measurements rendering;
	augs::time_measurements camera_query;
	augs::time_measurements gui;
	augs::time_measurements interpolation;
	augs::time_measurements visibility;
	augs::time_measurements physics_step;
	augs::time_measurements physics_readback;
	augs::time_measurements particles;
	augs::time_measurements ai;
	augs::time_measurements movement_paths;
	augs::time_measurements movement;
	augs::time_measurements stateful_animations;
	augs::time_measurements sentiences;

	augs::time_measurements deserialization_pass = 1;

	augs::time_measurements size_calculation_pass = 1;
	augs::time_measurements memory_allocation_pass = 1;
	augs::time_measurements serialization_pass = 1;

	augs::amount_measurements<std::size_t> delta_bytes = 1;

	augs::time_measurements duplication = 1;

	augs::time_measurements delta_encoding = 1;
	augs::time_measurements delta_decoding = 1;
	// END GEN INTROSPECTOR
};
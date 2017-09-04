#pragma once
#include "augs/misc/profiler_mixin.h"

class cosmic_profiler : public augs::profiler_mixin<cosmic_profiler> {
public:
	cosmic_profiler();

	// GEN INTROSPECTOR class cosmic_profiler
	augs::time_measurements complete_reinference = 1;

	augs::amount_measurements<std::size_t> raycasts;
	augs::amount_measurements<std::size_t> entropy_length;

	augs::time_measurements logic;
	augs::time_measurements rendering;
	augs::time_measurements camera_query;
	augs::time_measurements gui;
	augs::time_measurements interpolation;
	augs::time_measurements visibility;
	augs::time_measurements physics;
	augs::time_measurements particles;
	augs::time_measurements ai;
	augs::time_measurements pathfinding;

	augs::time_measurements total_load = 1;
	augs::time_measurements reading_savefile = 1;
	augs::time_measurements deserialization_pass = 1;

	augs::time_measurements total_save = 1;
	augs::time_measurements size_calculation_pass = 1;
	augs::time_measurements memory_allocation_pass = 1;
	augs::time_measurements serialization_pass = 1;
	augs::time_measurements writing_savefile = 1;

	augs::amount_measurements<std::size_t> delta_bytes = 1;

	augs::time_measurements duplication = 1;

	augs::time_measurements delta_encoding = 1;
	augs::time_measurements delta_decoding = 1;
	// END GEN INTROSPECTOR
};
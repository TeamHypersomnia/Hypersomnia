#pragma once
#include "augs/misc/enum/enum_boolset.h"
#include "game/enums/tree_of_npo_type.h"

struct tree_of_npo_filter {
	augs::enum_boolset<tree_of_npo_type> types;
	bool force_add_all_icons = false;

	static auto all() {
		tree_of_npo_filter result;
		fill_range(result.types, true);
		return result;
	}

	static auto all_visual() {
		auto result = all();
		result.types[tree_of_npo_type::SOUND_SOURCES] = false;
		return result;
	}

	static auto all_drawables() {
		tree_of_npo_filter result;

		result.types = {
			tree_of_npo_type::ORGANISMS,
			tree_of_npo_type::RENDERABLES
		};

		return result;
	}

	static auto all_and_force_add_all_icons() {
		auto result = all();
		result.force_add_all_icons = true;

		return result;
	}
};

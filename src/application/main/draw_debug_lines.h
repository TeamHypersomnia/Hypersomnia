#pragma once
#include "augs/graphics/renderer.h"
#include "augs/texture_atlas/atlas_entry.h"

struct camera_cone;
class cosmos;
struct config_json_table;

void draw_debug_lines(
	const cosmos& viewed_cosmos,
	augs::renderer& renderer,
	const real32 interpolation_ratio,
	const augs::atlas_entry default_texture,
	const config_json_table& cfg,
	const camera_cone cone
);

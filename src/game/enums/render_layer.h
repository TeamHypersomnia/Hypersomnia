#pragma once
/*

	TODO: Some render layers might correspond to distinct entity types.
	Rendering performance could be vastly improved by assigning an ordering based on these types,
	as we would avoid dispatching per-entity.
*/

enum class render_layer {
	// GEN INTROSPECTOR enum class render_layer
	INVALID,

	GROUND,
	GROUND_NEON_ERASER,
	PLANTED_BOMBS,
	SOLID_OBSTACLES,
	SOLID_OBSTACLES_OCCLUDING_NEONS,
	GLASS_OBSTACLES,
	DROPPED_ITEMS,
	SENTIENCES,
	FOREGROUND,
	GLOWING_FOREGROUND,
	DIM_WANDERING_PIXELS,
	CONTINUOUS_SOUNDS,
	CONTINUOUS_PARTICLES,
	ILLUMINATING_WANDERING_PIXELS,
	LIGHTS,
	AREA_MARKERS,
	POINT_MARKERS,
	CALLOUT_MARKERS,
	OVERLAID_CALLOUT_MARKERS,
	INSECTS,

	COUNT
	// END GEN INTROSPECTOR
};

enum class special_layer {
	GROUND_NEON_OCCLUDERS,

	COUNT
};

enum class builder_render_layer {
	GROUND,
	GROUND_NEON_ERASER,
	SOLID_OBSTACLES,
	GLASS_OBSTACLES,
	OVER_CHARACTERS,
	OVER_CHARACTERS_LIGHT,

	COUNT
};
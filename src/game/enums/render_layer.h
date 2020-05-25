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

	PLANTED_ITEMS,
	SOLID_OBSTACLES,
	ITEMS_ON_GROUND,
	SENTIENCES,
	FOREGROUND,
	FOREGROUND_GLOWS,

	DIM_WANDERING_PIXELS,
	CONTINUOUS_SOUNDS,
	CONTINUOUS_PARTICLES,
	ILLUMINATING_WANDERING_PIXELS,
	LIGHTS,
	AREA_MARKERS,
	POINT_MARKERS,
	CALLOUT_MARKERS,
	OVERLAID_CALLOUT_MARKERS,

	COUNT
	// END GEN INTROSPECTOR
};

enum class builder_render_layer {
	GROUND,
	SOLID_OBSTACLES,
	OVER_CHARACTERS,
	OVER_CHARACTERS_LIGHT,

	COUNT
};
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
	DYNAMIC_BODY,
	NEON_OCCLUDING_DYNAMIC_BODY,
	OVER_DYNAMIC_BODY,
	SMALL_DYNAMIC_BODY,
	OVER_SMALL_DYNAMIC_BODY,
	GLASS_BODY,
	SENTIENCES,
	OVER_SENTIENCES,
	FLYING_BULLETS,
	NEON_CAPTIONS,
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
#pragma once

/*
	Game balance constants that are baked into the build
	and cannot be overridden from config files.
*/

/*
	Camera zoom-out applied for balance: the default value
	of the per-map default_zoom arena setting. Maps can still
	override it, and camera_zoom areas take precedence locally.
*/
constexpr float BALANCE_ZOOM_OUT = 0.7f;

/*
	The extra camera zoom-out when the crosshair reaches the screen edge -
	the player then sees 1 / this farther.
*/
constexpr float BALANCE_MAX_ZOOM_OUT_AT_EDGES = 0.7f;

/*
	The fog of war angle narrows down to this fraction when fully zoomed out at the screen edge.
*/
constexpr float BALANCE_EDGE_ZOOM_OUT_FOW_ANGLE_MULT = 1.0f / 12;

/*
	The minimap pins an enemy the viewed character sees only if they are
	within this fraction of the character's field of view angle - so that
	enemies at the very edge of the view cone don't get pinned.
*/
constexpr float BALANCE_MINIMAP_SIGHTING_FOV_ANGLE_MULT = 0.75f;

/*
	Bots see this fraction of what a player in the same spot would see -
	both the range (the fog of war scaled by the camera zoom) and the view angle.
*/
constexpr float BALANCE_BOT_FOV_MULT_VERY_EASY = 0.8f;
constexpr float BALANCE_BOT_FOV_MULT_EASY = 0.85f;
constexpr float BALANCE_BOT_FOV_MULT_MEDIUM = 0.9f;
constexpr float BALANCE_BOT_FOV_MULT_HARD = 0.95f;

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
constexpr float BALANCE_ZOOM_OUT = 0.8f;

#pragma once

/*
	How the trajectories of grenades and melee weapons are drawn - see throw_trajectory_system:
	- SILHOUETTES of the item along the trajectory, a gap apart, over a dashed line,
	- LASER: a polyline like the laser of guns, with the dashed line from the first bounce to the crosshair.
	The silhouette where the throw ends is drawn in both.
*/

enum class throw_trajectory_mode {
	// GEN INTROSPECTOR enum class throw_trajectory_mode
	SILHOUETTES,
	LASER,
	COUNT
	// END GEN INTROSPECTOR
};

#pragma once
#include <vector>
#include "augs/graphics/debug_line.h"

/* Why it's global:
	1. Debug drawing is performed in many places in the logic, even in some very modular ones (force application). 
	If we stored it somehow in logic_step, we would need to pass step information to these modular functionalities as well,
	so that they may access the settings.
	This would be unnecessary headache.

	2. Access to a global variable is faster which is important in the logic,
	especially given that we might not want to use the debug drawing at all.

	3. We 'could' store it in augs::renderer, to which anyway global access is possible,
	but then we couple very game-specific flags with a structure that is just meant to be pushed triangles and lines into.
	Anyway, augs::renderer does not use that information at all.

	4. We couldn't care less. 
*/

struct debug_drawing_settings {
	// GEN INTROSPECTOR struct debug_drawing_settings
	bool enabled = false;

	bool draw_collinearization = false;
	bool draw_melee_info = false;
	bool draw_forces = false;
	bool draw_friction_field_collisions_of_entering = false;
	bool draw_explosion_forces = false;

	bool draw_triangle_edges = false;
	bool draw_cast_rays = false;
	bool draw_discontinuities = false;
	bool draw_visible_walls = false;

	bool draw_memorised_walls = false;
	bool draw_undiscovered_locations = false;
	bool draw_npo_tree_nodes = false;

	bool draw_camera_query = false;
	bool draw_headshot_detection = false;
	// END GEN INTROSPECTOR
};

extern debug_drawing_settings DEBUG_DRAWING;
extern std::vector<debug_line> DEBUG_LOGIC_STEP_LINES;
extern std::vector<debug_line> DEBUG_PERSISTENT_LINES;
extern std::vector<debug_line> DEBUG_FRAME_LINES;

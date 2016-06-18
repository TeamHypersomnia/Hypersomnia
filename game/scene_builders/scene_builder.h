#pragma once

class cosmos;
/* 
A scene builder is that which was inconvenient to put into system,
so a dummy testbed setup for example.

In these classes you will find some traffic meters, whatever thing that does not deserve its component+system pair.

A scene builder cannot or need not be composed with other scene builders.

*/
#include "game/messages/camera_render_request_message.h"

struct scene_builder {
	virtual void load_resources();
	virtual void populate_world_with_entities(cosmos&);
	virtual void perform_logic_step(cosmos&);
	virtual void drawcalls_after_all_cameras(cosmos&);

	virtual void execute_drawcalls_for_camera(messages::camera_render_request_message);
};

#include "animation.h"

animation::frame::frame(renderable* instance, float duration_milliseconds) 
	: instance(instance), duration_milliseconds(duration_milliseconds) {}

animation::animation() : loop_mode(loop_type::REPEAT) {}

void animation::add_frame(renderable* instance, float duration_milliseconds) {
	frames.push_back(frame(instance, duration_milliseconds));
}

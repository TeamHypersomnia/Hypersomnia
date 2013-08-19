#include "animation_system.h"
#include "entity_system/world.h"
#include "../messages/animate_message.h"

#include "../animation.h"
using namespace messages;

void animation_system::process_entities(world& owner) {
	auto events = owner.get_message_queue<animate_message>();

	for (auto it : events) {
		auto& animate = it.subject->get<components::animate>();

		switch (it.message_type) {
		case animate_message::type::PAUSE:
			animate.enabled = false;
			break;
		case animate_message::type::STOP:
			animate.enabled = false;
			animate.current_frame = 0;
			animate.current_ms = 0.f;
			break;
		case animate_message::type::START:
			animate.enabled = true;
			animate.current_frame = 0;
			animate.current_ms = 0.f;
			break;
		case animate_message::type::CONTINUE:
			animate.enabled = true;
			break;
		default: break;
		}

		if (it.change_speed)
			animate.speed_factor = it.speed_factor;

		if (it.change_animation) {
			auto new_instance = animate.available_animations.get(components::animate::response(it.animation_type));

			if (new_instance->instance != animate.current_animation) {
				if (!it.preserve_state) {
					animate.current_frame = 0;
					animate.current_ms = 0.f;
				}
				new_instance->instance = animate.current_animation;
			}
		}
	}

	float delta = static_cast<float>(animation_timer.extract<std::chrono::milliseconds>());

	for (auto it : targets) {
		auto& animate = it->get<components::animate>();
		auto& render = it->get<components::render>();
		auto& animation = *animate.current_animation;

		if (animate.enabled) {
			animate.current_ms += delta * animate.speed_factor;

			while (true) {
				float frame_duration = animation.frames[animate.current_frame].duration_milliseconds;

				if (animate.current_ms > frame_duration) {
					animate.current_ms -= frame_duration;

					if (animation.loop_mode == animation::loop_type::INVERSE) {
						if (animate.increasing) {
							if (animate.current_frame < animation.frames.size() - 1) ++animate.current_frame;
							else {
								--animate.current_frame;
								animate.increasing = false;
							}
						}
						else {
							if (animate.current_frame > 0) --animate.current_frame;
							else {
								++animate.current_frame;
								animate.increasing = true;
							}
						}
					}
					else if (animation.loop_mode == animation::loop_type::REPEAT) {
						if (animate.current_frame < animation.frames.size()-1)
							++animate.current_frame;
						else animate.current_frame = 0;
					}
					else if (animation.loop_mode == animation::loop_type::NONE) {
						if (animate.current_frame < animation.frames.size()-1)
							++animate.current_frame;
					}
				}
				else break;
			}
		}

		render.instance = animation.frames[animate.current_frame].instance;
	}
}

#include "animation_system.h"
#include "entity_system/world.h"
#include "../messages/animate_message.h"

#include "../animation.h"
using namespace messages;

void animation_system::process_entities(world& owner) {
	auto events = owner.get_message_queue<animate_message>();

	for (auto it : events) {
		auto ptr = it.subject->find<components::animate>();
		if (!ptr) continue; auto& animate = *ptr;

		if (it.animation_priority >= animate.current_priority || animate.current_state == components::animate::state::PAUSED) {
			animate.current_priority = it.animation_priority;

			switch (it.message_type) {
			case animate_message::type::PAUSE:
				if (animate.current_state != components::animate::state::PAUSED) {
					animate.paused_state = animate.current_state;
					animate.current_state = components::animate::state::PAUSED;
				}
				break;
			case animate_message::type::STOP:
				animate.paused_state = components::animate::state::INCREASING;
				animate.current_state = components::animate::state::PAUSED;
				animate.current_frame = 0;
				animate.current_ms = 0.f;
				break;
			case animate_message::type::START:
				animate.current_state = components::animate::state::INCREASING;
				animate.current_frame = 0;
				animate.current_ms = 0.f;
				break;
			case animate_message::type::CONTINUE:
				if (animate.current_state == components::animate::state::PAUSED) {
					animate.current_state = animate.paused_state;
				}
				break;
			default: break;
			}

			if (it.change_speed)
				animate.speed_factor = it.speed_factor;

			if (it.change_animation) {
				auto new_instance = animate.available_animations->at(it.animation_type);

				if (new_instance != animate.current_animation) {
					if (!it.preserve_state) {
						animate.current_frame = 0;
						animate.current_ms = 0.f;
					}
					animate.current_animation = new_instance;
				}
			}
		}
	}

	float delta = static_cast<float>(animation_timer.extract<std::chrono::milliseconds>());

	for (auto it : targets) {
		auto& animate = it->get<components::animate>();
		auto& render = it->get<components::render>();
		auto& animation = *animate.current_animation;
		
		if (animate.current_state != components::animate::state::PAUSED) {
			animate.current_ms += delta * animate.speed_factor;

			while (true) {
				float frame_duration = animation.frames[animate.current_frame].duration_milliseconds;

				if (animate.current_ms > frame_duration) {
					animate.current_ms -= frame_duration;

					if (animation.loop_mode == animation::loop_type::INVERSE) {

						if (animate.current_state == components::animate::state::INCREASING) {
							if (animate.current_frame < animation.frames.size() - 1) ++animate.current_frame;
							else {
								--animate.current_frame;
								animate.current_state = components::animate::state::DECREASING;
							}
						}

						else if (animate.current_state == components::animate::state::DECREASING) {
							if (animate.current_frame > 0) --animate.current_frame;
							else {
								++animate.current_frame;
								animate.current_state = components::animate::state::INCREASING;
							}
						}
					}

					else if (animation.loop_mode == animation::loop_type::REPEAT) {
						if (animate.current_state == components::animate::state::INCREASING) {
							if (animate.current_frame < animation.frames.size() - 1)
								++animate.current_frame;
							else animate.current_frame = 0;
						}
						else if (animate.current_state == components::animate::state::DECREASING) {
							if (animate.current_frame > 0) --animate.current_frame;
							else animate.current_frame = animation.frames.size() - 1;
						}
					}

					else if (animation.loop_mode == animation::loop_type::NONE) {
						if (animate.current_state == components::animate::state::INCREASING) {
							if (animate.current_frame < animation.frames.size() - 1)
								++animate.current_frame;
							else animate.current_state = components::animate::state::PAUSED;
						}
						else if (animate.current_state == components::animate::state::DECREASING) {
							if (animate.current_frame > 0) --animate.current_frame;
							else animate.current_state = components::animate::state::PAUSED;
						}
					}
				}
				else break;
			}
		}

		render.instance = animation.frames[animate.current_frame].instance;
	}
}

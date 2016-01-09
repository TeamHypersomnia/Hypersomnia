#include "animation_system.h"
#include "entity_system/world.h"
#include "../messages/animation_message.h"
#include <iostream>

using namespace messages;
using namespace resources;

void animation_system::consume_events() {
	auto events = parent_world.get_message_queue<animation_message>();

	for (auto it : events) {
		auto ptr = it.subject->find<components::animation>();
		if (!ptr) continue; auto& animation = *ptr;

		if (it.animation_priority >= animation.current_priority || animation.current_state == components::animation::state::PAUSED) {
			if (it.change_speed)
				animation.speed_factor = it.speed_factor;

			//if (it.change_animation) {
				resources::animation* new_instance = nullptr;
				if (it.set_animation)
					new_instance = it.set_animation;
				else {
					auto found_animation = animation.available_animations->get_raw().find(it.animation_type);
					if (found_animation != animation.available_animations->get_raw().end()) {
						new_instance = (*found_animation).second;
					}
				}

				if (new_instance != animation.current_animation) {
					animation.current_animation = new_instance;

					if (it.message_type == animation_message::action::CONTINUE) {
						it.message_type = animation_message::action::START;
					}
					//if (!it.preserve_state_if_animation_changes) {
					//	animation.set_current_frame(0, it.subject);
					//	animation.current_ms = 0.f;
					//}
					//else {
						/* update callback */
						//animation.set_current_frame(animation.get_current_frame(), it.subject);
					//}
				}
			//}
			
			animation.current_priority = it.animation_priority;

			switch (it.message_type) {
			case animation_message::action::PAUSE:
				if (animation.current_state != components::animation::state::PAUSED) {
					animation.paused_state = animation.current_state;
					animation.current_state = components::animation::state::PAUSED;
				}
				break;
			case animation_message::action::STOP:
				animation.paused_state = components::animation::state::INCREASING;
				animation.current_state = components::animation::state::PAUSED;
				animation.set_current_frame(0, it.subject, false);
				animation.current_ms = 0.f;
				break;
			case animation_message::action::START:
				animation.current_state = components::animation::state::INCREASING;
				animation.set_current_frame(0, it.subject);
				animation.current_ms = 0.f;
				break;
			case animation_message::action::CONTINUE:
				if (animation.current_state == components::animation::state::PAUSED) {
					animation.current_state = animation.paused_state;
				}
				break;
			default: break;
			}
		}
	}
}

void call(animation_callback func, augs::entity_id subject) {
	if (func) 
		func(subject);
}

void components::animation::set_current_frame(unsigned number, augs::entity_id subject, bool do_callback) {
	if (saved_callback_out)
		call(saved_callback_out, subject);
	
	current_frame = number;
	if (current_animation) {
		if (do_callback) {
			call(current_animation->frames[current_frame].callback, subject);
			saved_callback_out = current_animation->frames[current_frame].callback_out;
		}

		subject->get<components::sprite>() = current_animation->frames[get_current_frame()].sprite;
	} 
	else
		saved_callback_out = nullptr;
}

void components::animation::set_current_animation_set(resources::animation_info* set, augs::entity_id subject) {
	if (saved_callback_out) {
		call(saved_callback_out, subject);
	}
	available_animations = set;
	current_frame = 0;
	saved_callback_out = nullptr;

	paused_state = components::animation::state::INCREASING;
	current_state = components::animation::state::PAUSED;
	current_ms = 0.f;
	current_animation = nullptr;

	set_current_frame(0, subject);
}

void animation_system::process_entities() {
	float delta = static_cast<float>(animation_timer.extract<std::chrono::milliseconds>());

	for (auto it : targets) {
		auto& animation_state = it->get<components::animation>();

		if (animation_state.current_animation == nullptr) continue;

		auto& animation = *animation_state.current_animation;
		if (animation.frames.empty()) continue;

		if (animation_state.current_state != components::animation::state::PAUSED) {
			animation_state.current_ms += delta * animation_state.speed_factor;

			while (true) {
				float frame_duration = animation.frames[animation_state.get_current_frame()].duration_milliseconds;

				if (animation_state.current_ms > frame_duration) {
					animation_state.current_ms -= frame_duration;

					if (animation.loop_mode == animation::loop_type::INVERSE) {

						if (animation_state.current_state == components::animation::state::INCREASING) {
							if (animation_state.get_current_frame() < animation.frames.size() - 1) animation_state.increase_frame(it);
							else {
								animation_state.decrease_frame(it);
								animation_state.current_state = components::animation::state::DECREASING;
							}
						}

						else if (animation_state.current_state == components::animation::state::DECREASING) {
							if (animation_state.get_current_frame() > 0) animation_state.decrease_frame(it);
							else {
								animation_state.increase_frame(it);
								animation_state.current_state = components::animation::state::INCREASING;
							}
						}
					}

					else if (animation.loop_mode == animation::loop_type::REPEAT) {
						if (animation_state.current_state == components::animation::state::INCREASING) {
							if (animation_state.get_current_frame() < animation.frames.size() - 1)
								animation_state.increase_frame(it);
							else animation_state.set_current_frame(0, it);
						}
						else if (animation_state.current_state == components::animation::state::DECREASING) {
							if (animation_state.get_current_frame() > 0) animation_state.decrease_frame(it);
							else animation_state.set_current_frame(animation.frames.size() - 1, it);
						}
					}

					else if (animation.loop_mode == animation::loop_type::NONE) {
						if (animation_state.current_state == components::animation::state::INCREASING) {
							if (animation_state.get_current_frame() < animation.frames.size() - 1)
								animation_state.increase_frame(it);
							else animation_state.current_state = components::animation::state::PAUSED;
						}
						else if (animation_state.current_state == components::animation::state::DECREASING) {
							if (animation_state.get_current_frame() > 0) animation_state.decrease_frame(it);
							else animation_state.current_state = components::animation::state::PAUSED;
						}
					}
				}
				else break;
			}
		}

		auto& sprite = it->get<components::sprite>();
		sprite = animation.frames[animation_state.get_current_frame()].sprite;
	}
}

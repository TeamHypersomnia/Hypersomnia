#include "animation_system.h"
#include "entity_system/world.h"
#include "../components/animation_response_component.h"
#include "../messages/movement_response.h"
#include "../messages/animation_message.h"

#include "../resources/manager.h"

using namespace messages;
using namespace resources;

void animation_system::game_responses_to_animation_messages() {
	auto& movements = parent_world.get_message_queue<movement_response>();
	// auto& gunshots = parent_world.get_message_queue<movement_response>();

	for (auto it : movements) {
		animation_message msg;

		msg.subject = it.subject;
		msg.change_speed = true;

		msg.change_animation = true;
		msg.preserve_state_if_animation_changes = false;
		msg.action = ((it.speed <= 1.f) ? animation_message::STOP : animation_message::CONTINUE);

		if (!it.stop_response_at_zero_speed)
			msg.action = animation_message::CONTINUE;

		msg.animation_priority = 0;

		msg.set_animation = (*(it.subject->get<components::animation_response>().response))[animation_response_type::MOVE];
		msg.speed_factor = it.speed;

		parent_world.post_message(msg);
	}
}

void animation_system::handle_animation_messages() {
	auto events = parent_world.get_message_queue<animation_message>();

	for (auto it : events) {
		auto ptr = it.subject->find<components::animation>();
		if (!ptr) continue; auto& animation = *ptr;

		if (it.animation_priority >= animation.priority || animation.state == components::animation::playing_state::PAUSED) {
			if (it.change_speed)
				animation.speed_factor = it.speed_factor;

			//if (it.change_animation) {
				assets::animation_id new_instance = it.set_animation;
				
				if (new_instance != animation.current_animation) {
					animation.current_animation = new_instance;

					if (it.action == animation_message::CONTINUE) {
						it.action = animation_message::START;
					}
					//if (!it.preserve_state_if_animation_changes) {
					//	animation.set_frame_num(0, it.subject);
					//	animation.player_position_ms = 0.f;
					//}
					//else {
						/* update callback */
						//animation.set_frame_num(animation.get_frame_num(), it.subject);
					//}
				}
			//}
			
			animation.priority = it.animation_priority;

			switch (it.action) {
			case animation_message::PAUSE:
				if (animation.state != components::animation::playing_state::PAUSED) {
					animation.paused_state = animation.state;
					animation.state = components::animation::playing_state::PAUSED;
				}
				break;
			case animation_message::STOP:
				animation.paused_state = components::animation::playing_state::PAUSED;
				animation.state = components::animation::playing_state::PAUSED;
				animation.set_frame_num(0, it.subject, false);
				animation.player_position_ms = 0.f;
				break;
			case animation_message::START:
				animation.state = components::animation::playing_state::INCREASING;
				animation.set_frame_num(0, it.subject);
				animation.player_position_ms = 0.f;
				break;
			case animation_message::CONTINUE:
				if (animation.state == components::animation::playing_state::PAUSED) {
					if (animation.paused_state == components::animation::playing_state::PAUSED)
						animation.paused_state = components::animation::playing_state::INCREASING;

					animation.state = animation.paused_state;
				}
				break;
			default: break;
			}
		}
	}

	parent_world.get_message_queue<animation_message>().clear();
}

void call(animation_callback func, augs::entity_id subject) {
	if (func) 
		func(subject);
}

void components::animation::set_frame_num(unsigned number, augs::entity_id subject, bool do_callback) {
	if (saved_callback_out)
		call(saved_callback_out, subject);
	
	frame_num = number;

	const auto& animation_resource = *resource_manager.find(current_animation);

	if (do_callback) {
		call(animation_resource.frames[frame_num].callback, subject);
		saved_callback_out = animation_resource.frames[frame_num].callback_out;
	}

	subject->get<components::sprite>() = animation_resource.frames[get_frame_num()].sprite;
}

//void components::animation::set_current_animation_set(resources::animation_info* set, augs::entity_id subject) {
//	if (saved_callback_out) {
//		call(saved_callback_out, subject);
//	}
//	available_animations = set;
//	current_frame = 0;
//	saved_callback_out = nullptr;
//
//	paused_state = components::animation::state::INCREASING;
//	current_state = components::animation::state::PAUSED;
//	player_position_ms = 0.f;
//	current_animation = nullptr;
//
//	set_frame_num(0, subject);
//}

void animation_system::progress_animation_states() {
	float delta = static_cast<float>(animation_timer.extract<std::chrono::milliseconds>());

	for (auto it : targets) {
		auto& current = it->get<components::animation>();

		if (current.state != components::animation::playing_state::PAUSED) {
			auto& animation = *resource_manager.find(current.current_animation);

			if (animation.frames.empty()) continue;

			current.player_position_ms += delta * current.speed_factor;

			while (true) {
				float frame_duration = animation.frames[current.get_frame_num()].duration_milliseconds;

				if (current.player_position_ms > frame_duration) {
					current.player_position_ms -= frame_duration;

					if (animation.loop_mode == animation::loop_type::INVERSE) {

						if (current.state == components::animation::playing_state::INCREASING) {
							if (current.get_frame_num() < animation.frames.size() - 1) current.increase_frame(it);
							else {
								current.decrease_frame(it);
								current.state = components::animation::playing_state::DECREASING;
							}
						}

						else if (current.state == components::animation::playing_state::DECREASING) {
							if (current.get_frame_num() > 0) current.decrease_frame(it);
							else {
								current.increase_frame(it);
								current.state = components::animation::playing_state::INCREASING;
							}
						}
					}

					else if (animation.loop_mode == animation::loop_type::REPEAT) {
						if (current.state == components::animation::playing_state::INCREASING) {
							if (current.get_frame_num() < animation.frames.size() - 1)
								current.increase_frame(it);
							else current.set_frame_num(0, it);
						}
						else if (current.state == components::animation::playing_state::DECREASING) {
							if (current.get_frame_num() > 0) current.decrease_frame(it);
							else current.set_frame_num(animation.frames.size() - 1, it);
						}
					}

					else if (animation.loop_mode == animation::loop_type::NONE) {
						if (current.state == components::animation::playing_state::INCREASING) {
							if (current.get_frame_num() < animation.frames.size() - 1)
								current.increase_frame(it);
							else current.state = components::animation::playing_state::PAUSED;
						}
						else if (current.state == components::animation::playing_state::DECREASING) {
							if (current.get_frame_num() > 0) current.decrease_frame(it);
							else current.state = components::animation::playing_state::PAUSED;
						}
					}
				}
				else break;
			}

			auto& sprite = it->get<components::sprite>();
			sprite = animation.frames[current.get_frame_num()].sprite;
		}
	}
}

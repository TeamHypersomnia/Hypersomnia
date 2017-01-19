#include "animation_system.h"
#include "game/transcendental/cosmos.h"
#include "game/components/animation_response_component.h"
#include "game/messages/movement_response.h"
#include "game/messages/animation_message.h"
#include "game/messages/gunshot_response.h"

#include "game/resources/manager.h"

#include "game/components/animation_component.h"
#include "game/components/render_component.h"

#include "game/transcendental/entity_handle.h"
#include "game/transcendental/step.h"

using namespace augs;

using namespace messages;
using namespace resources;

void animation_system::game_responses_to_animation_messages(const logic_step step) {
	auto& cosmos = step.cosm;
	const auto& delta = step.get_delta();
	const auto& movements = step.transient.messages.get_queue<movement_response>();
	const auto& gunshots = step.transient.messages.get_queue<gunshot_response>();

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

		msg.set_animation = (*(cosmos[it.subject].get<components::animation_response>().response))[animation_response_type::MOVE];
		msg.speed_factor = it.speed;

		step.transient.messages.post(msg);
	}

	for (auto it : gunshots) {
		// animation_message msg;
		// msg.preserve_state_if_animation_changes = false;
		// msg.change_animation = true;
		// msg.change_speed = true;
		// msg.speed_factor = 1.f;
		// msg.subject = it.subject;
		// msg.action = messages::animation_message::START;
		// msg.animation_priority = 1;
		// msg.set_animation = (*(it.subject.get<components::animation_response>().response))[animation_response_type::SHOT];
		// 
		// step.transient.messages.post(msg);
	}
}

void animation_system::handle_animation_messages(const logic_step step) {
	auto& cosmos = step.cosm;
	const auto& delta = step.get_delta();
	const auto& events = step.transient.messages.get_queue<animation_message>();

	for (auto it : events) {
		auto ptr = cosmos[it.subject].find<components::animation>();
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
					//	animation.set_current_frame(0, it.subject);
					//	animation.player_position_ms = 0.f;
					//}
					//else {
						/* update callback */
						//animation.set_current_frame(animation.get_current_frame(), it.subject);
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
				animation.set_current_frame(0);
				animation.player_position_ms = 0.f;
				break;
			case animation_message::START:
				animation.state = components::animation::playing_state::INCREASING;
				animation.set_current_frame(0);
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

	step.transient.messages.get_queue<animation_message>().clear();
}

void components::animation::set_current_frame(unsigned number) {
	frame_num = number;
}

void animation_system::progress_animation_states(const logic_step step) {
	auto& cosmos = step.cosm;
	const auto& delta = step.get_delta();

	for (const auto& it : cosmos.get(processing_subjects::WITH_ANIMATION)) {
		auto& animation_state = it.get<components::animation>();

		if (animation_state.state != components::animation::playing_state::PAUSED) {
			auto& animation = *get_resource_manager().find(animation_state.current_animation);

			if (animation.frames.empty()) continue;

			animation_state.player_position_ms += static_cast<float>(delta.in_milliseconds()) * animation_state.speed_factor;

			while (true) {
				float frame_duration = animation.frames[animation_state.get_current_frame()].duration_milliseconds;

				if (animation_state.player_position_ms > frame_duration) {
					animation_state.player_position_ms -= frame_duration;

					if (animation.loop_mode == animation::loop_type::INVERSE) {

						if (animation_state.state == components::animation::playing_state::INCREASING) {
							if (animation_state.get_current_frame() < animation.frames.size() - 1) animation_state.increase_frame();
							else {
								animation_state.decrease_frame();
								animation_state.state = components::animation::playing_state::DECREASING;
							}
						}

						else if (animation_state.state == components::animation::playing_state::DECREASING) {
							if (animation_state.get_current_frame() > 0) animation_state.decrease_frame();
							else {
								animation_state.increase_frame();
								animation_state.state = components::animation::playing_state::INCREASING;
							}
						}
					}

					else if (animation.loop_mode == animation::loop_type::REPEAT) {
						if (animation_state.state == components::animation::playing_state::INCREASING) {
							if (animation_state.get_current_frame() < animation.frames.size() - 1)
								animation_state.increase_frame();
							else animation_state.set_current_frame(0);
						}
						else if (animation_state.state == components::animation::playing_state::DECREASING) {
							if (animation_state.get_current_frame() > 0) animation_state.decrease_frame();
							else animation_state.set_current_frame(animation.frames.size() - 1);
						}
					}

					else if (animation.loop_mode == animation::loop_type::NONE) {
						if (animation_state.state == components::animation::playing_state::INCREASING) {
							if (animation_state.get_current_frame() < animation.frames.size() - 1)
								animation_state.increase_frame();
							else animation_state.state = components::animation::playing_state::PAUSED;
						}
						else if (animation_state.state == components::animation::playing_state::DECREASING) {
							if (animation_state.get_current_frame() > 0) animation_state.decrease_frame();
							else animation_state.state = components::animation::playing_state::PAUSED;
						}
					}
				}
				else break;
			}

			auto& sprite = it.get<components::sprite>();
			sprite = animation.frames[animation_state.get_current_frame()].sprite;
		}
	}
}

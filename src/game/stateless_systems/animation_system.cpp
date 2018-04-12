#include "animation_system.h"

#include "game/transcendental/cosmos.h"

#include "game/messages/animation_message.h"
#include "game/messages/gunshot_response.h"

#include "game/components/animation_component.h"
#include "game/components/render_component.h"

#include "game/transcendental/entity_handle.h"
#include "game/transcendental/logic_step.h"
#include "game/transcendental/data_living_one_step.h"
#include "game/assets/all_logical_assets.h"

using namespace augs;

using namespace messages;

void animation_system::game_responses_to_animation_messages(const logic_step step) {}

void animation_system::handle_animation_messages(const logic_step step) {
	auto& cosmos = step.get_cosmos();
	const auto& events = step.get_queue<animation_message>();

	for (auto it : events) {
		auto ptr = cosmos[it.subject].find<components::animation>();
		if (!ptr) continue; auto& animation = *ptr;

		if (it.animation_priority >= animation.priority || animation.state == animation_playing_state::PAUSED) {
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
				if (animation.state != animation_playing_state::PAUSED) {
					animation.paused_state = animation.state;
					animation.state = animation_playing_state::PAUSED;
				}
				break;
			case animation_message::STOP:
				animation.paused_state = animation_playing_state::PAUSED;
				animation.state = animation_playing_state::PAUSED;
				animation.set_current_frame(0);
				animation.player_position_ms = 0.f;
				break;
			case animation_message::START:
				animation.state = animation_playing_state::INCREASING;
				animation.set_current_frame(0);
				animation.player_position_ms = 0.f;
				break;
			case animation_message::CONTINUE:
				if (animation.state == animation_playing_state::PAUSED) {
					if (animation.paused_state == animation_playing_state::PAUSED)
						animation.paused_state = animation_playing_state::INCREASING;

					animation.state = animation.paused_state;
				}
				break;
			default: break;
			}
		}
	}
}

void components::animation::set_current_frame(const unsigned number) {
	frame_num = number;
}

void animation_system::progress_animation_states(const logic_step step) {
	auto& cosmos = step.get_cosmos();
	const auto& metas = step.get_logical_assets().animations;
	const auto& delta = step.get_delta();

	cosmos.for_each_having<components::animation>(
		[&](const auto it) {
			auto& animation_state = it.template get<components::animation>();

			if (animation_state.state != animation_playing_state::PAUSED) {
				auto& animation = metas.at(animation_state.current_animation);

				if (animation.frames.empty()) {
					return;
				}

				animation_state.player_position_ms += static_cast<float>(delta.in_milliseconds()) * animation_state.speed_factor;

				while (true) {
					float frame_duration = animation.frames[animation_state.get_current_frame()].duration_milliseconds;

					if (animation_state.player_position_ms > frame_duration) {
						animation_state.player_position_ms -= frame_duration;

						if (animation.loop_mode == animation::loop_type::INVERSE) {

							if (animation_state.state == animation_playing_state::INCREASING) {
								if (animation_state.get_current_frame() < animation.frames.size() - 1) animation_state.increase_frame();
								else {
									animation_state.decrease_frame();
									animation_state.state = animation_playing_state::DECREASING;
								}
							}

							else if (animation_state.state == animation_playing_state::DECREASING) {
								if (animation_state.get_current_frame() > 0) animation_state.decrease_frame();
								else {
									animation_state.increase_frame();
									animation_state.state = animation_playing_state::INCREASING;
								}
							}
						}

						else if (animation.loop_mode == animation::loop_type::REPEAT) {
							if (animation_state.state == animation_playing_state::INCREASING) {
								if (animation_state.get_current_frame() < animation.frames.size() - 1)
									animation_state.increase_frame();
								else animation_state.set_current_frame(0);
							}
							else if (animation_state.state == animation_playing_state::DECREASING) {
								if (animation_state.get_current_frame() > 0) animation_state.decrease_frame();
								else animation_state.set_current_frame(static_cast<unsigned>(animation.frames.size()) - 1);
							}
						}

						else if (animation.loop_mode == animation::loop_type::NONE) {
							if (animation_state.state == animation_playing_state::INCREASING) {
								if (animation_state.get_current_frame() < animation.frames.size() - 1)
									animation_state.increase_frame();
								else animation_state.state = animation_playing_state::PAUSED;
							}
							else if (animation_state.state == animation_playing_state::DECREASING) {
								if (animation_state.get_current_frame() > 0) animation_state.decrease_frame();
								else animation_state.state = animation_playing_state::PAUSED;
							}
						}
					}
					else break;
				}

#if TODO
				// TODO: stateless calculation

				auto& sprite = it.template get<components::sprite>();

				sprite.set(
					animation.frames[animation_state.get_current_frame()].image_id,
					metas
				);
#endif
			}
		}
	);
}

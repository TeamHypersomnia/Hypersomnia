#pragma once
#include "game/components/gun_component.h"
#include "game/components/movement_component.h"
#include "game/components/melee_fighter_component.h"
#include "game/assets/animation_math.h"
#include "game/detail/inventory/weapon_reloading.hpp"
#include "game/assets/animation.h"
#include "game/components/animation_component.h"

using cosmos_clock = augs::stepped_clock;

template <class L>
const plain_animation_frame* find_action_frame(
	const stance_animations& stance,
	const components::melee_fighter& fighter,
	const L& logicals
) {
	const auto s = fighter.state;

	const auto chosen_anim = [&]() {
		if (s == melee_fighter_state::IN_ACTION) {
			return stance.actions[fighter.action].perform;
		}

		if (fighter.now_returning()) {
			return stance.actions[fighter.action].returner;
		}

		return assets::plain_animation_id();
	}();

	if (const auto action_animation = logicals.find(chosen_anim)) {
		const auto& f = action_animation->frames;
		const auto n = static_cast<unsigned>(f.size());
		const auto i = std::min(n - 1, fighter.anim_state.frame_num);

		return f.data() + i;
	}

	return nullptr;
}

template <class L>
std::pair<const plain_animation_frame*, const plain_animation_frame*> find_first_and_current_frame(
	const assets::plain_animation_id& anim_id,
	const components::animation& anim_state,
	const L& logicals
) {
	if (const auto anim = logicals.find(anim_id)) {
		const auto n = static_cast<unsigned>(anim->frames.size());
		return {
			/* Return the reference frame too e.g. for size mult calculation */
			std::addressof(anim->frames[0]),
			std::addressof(anim->frames[anim_state.state.frame_num % n])
		};
	}

	return { nullptr, nullptr };
}

template <class L>
decltype(auto) find_first_and_current_frame(
	const invariants::animation& anim_def,
	const components::animation& anim_state,
	const L& logicals
) {
	return find_first_and_current_frame(anim_def.id, anim_state, logicals);
}

template <class L>
const plain_animation_frame* find_frame(
	const assets::plain_animation_id& anim_id,
	const components::animation& anim_state,
	const L& logicals
) {
	if (const auto anim = logicals.find(anim_id)) {
		const auto n = static_cast<unsigned>(anim->frames.size());
		return std::addressof(anim->frames[anim_state.state.frame_num % n]);
	}

	return nullptr;
}

template <class L>
decltype(auto) find_frame(
	const invariants::animation& anim_def,
	const components::animation& anim_state,
	const L& logicals
) {
	return find_frame(anim_def.id, anim_state, logicals);
}

template <class T>
frame_type_t<T>* find_shoot_frame(
	const invariants::gun& gun_def,
	const components::gun& gun,
	T& anim, 
	const cosmos_clock& clk
) {
	return calc_current_frame(anim, clk.get_passed_ms(gun_def.shot_cooldown_ms, gun.fire_cooldown_object));
}

template <class T, class C>
frame_type_t<T>* find_shoot_frame(
	const invariants::gun& gun_def,
	const components::gun& gun,
	T& anim, 
	const C& cosm
) {
	return ::find_shoot_frame(gun_def, gun, anim, cosm.get_clock());
}

template <class T>
frame_type_t<T>* find_chambering_frame(
	const components::gun& gun,
	T& anim
) {
	const auto animation_time_ms = gun.chambering_progress_ms / 1.3f;

	if (animation_time_ms == 0.f) {
		return nullptr;
	}

	return calc_current_frame(anim, animation_time_ms);
}

template <class T>
struct frame_and_flip {
	frame_type_t<T>& frame;
	flip_flags flip;
};

template <class T>
frame_and_flip<T> get_frame_and_flip(
	const movement_animation_state& state, 
	T& animation
) {
	const auto animation_frames_n = static_cast<unsigned>(animation.frames.size());
	const auto index = state.get_multi_way_index(animation_frames_n);

	return { 
		animation.frames[index % animation_frames_n], 
		state.flip ? animation.meta.flip_when_cycling : flip_flags()
	};
}

enum class stance_flag {
	SHOOTING,
	COUNT
};

template <class T>
struct stance_frame_usage {
	const frame_type_t<T>* const frame;
	flip_flags movement_flip;
	const augs::enum_boolset<stance_flag> flags;

	static auto none() {
		return stance_frame_usage<T> { nullptr, flip_flags(), {} };
	}

	static auto carry(const frame_and_flip<T>& f) {
		return stance_frame_usage<T> { std::addressof(f.frame), f.flip, {} };
	}

	static auto grip_to_mag(const frame_type_t<T>& f) {
		return stance_frame_usage<T> { std::addressof(f), flip_flags(), {} };
	}

	static auto pocket_to_mag(const frame_type_t<T>& f) {
		return stance_frame_usage<T> { std::addressof(f), flip_flags(), {} };
	}

	static auto shoot(const frame_type_t<T>& f) {
		return stance_frame_usage<T> { std::addressof(f), flip_flags(), { stance_flag::SHOOTING } };
	}

	static auto chambering(const frame_type_t<T>& f) {
		return stance_frame_usage<T> { std::addressof(f), flip_flags(), { stance_flag::SHOOTING } };
	}

	static auto shoot_flipped(const frame_type_t<T>& f) {
		return stance_frame_usage<T> { std::addressof(f), flip_flags::make_vertically(), { stance_flag::SHOOTING } };
	}

	explicit operator bool() const {
		return frame != nullptr;
	}

	auto get_with_flip() const {
		return frame_and_flip<T>{ *frame, movement_flip };
	}
};

template <class E, class T>
auto calc_stance_usage(
	const E& typed_handle,
	const stance_animations& stance,
	const movement_animation_state& anim_state, 
	const T& wielded_items
) {
	using result_t = stance_frame_usage<const torso_animation>;

	const auto& cosm = typed_handle.get_cosmos();
	const auto& logicals = cosm.get_logical_assets();

	const auto n = wielded_items.size();

	if (n > 0) {
		if (const auto rld = ::calc_reloading_movement(cosm, wielded_items)) {
			const bool gtm = rld->type == reloading_movement_type::GRIP_TO_MAG;
			const auto anim = 
				gtm
				? stance.grip_to_mag
				: stance.pocket_to_mag
			;

			if (const auto anim_ptr = logicals.find(anim)) {
				if (const auto found_frame = ::calc_current_frame(*anim_ptr, rld->progress_ms)) {
					if (gtm) {
						return result_t::grip_to_mag(*found_frame);
					}

					return result_t::pocket_to_mag(*found_frame);
				}
			}
		}

		if (const auto gun = cosm[wielded_items[0]].template find<components::gun>()) {
			if (const auto shoot_animation = logicals.find(stance.actions[weapon_action_type::PRIMARY].perform)) {
				const auto frame = ::find_shoot_frame(cosm[wielded_items[0]].template get<invariants::gun>(), *gun, *shoot_animation, cosm);
				const auto second_frame = [n, &cosm, shoot_animation, &wielded_items]() -> decltype(frame) {
					if (n == 2) {
						if (const auto second_gun = cosm[wielded_items[1]].template find<components::gun>()) {
							return ::find_shoot_frame(cosm[wielded_items[1]].template get<invariants::gun>(), *second_gun, *shoot_animation, cosm);
						}
					}

					return nullptr;
				}();

				if (frame && second_frame) {
					if (frame < second_frame) {
						return result_t::shoot(*frame);
					}

					return result_t::shoot_flipped(*second_frame);
				}

				if (frame) {
					return result_t::shoot(*frame);
				}

				if (second_frame) {
					return result_t::shoot_flipped(*second_frame);
				}
			}
		}

		if (const auto fighter = typed_handle.template find<components::melee_fighter>()) {
			if (const auto frame = find_action_frame(stance, *fighter, logicals)) {
				return result_t::shoot(*frame);
			}
		}
	}

	if (n == 1) {
		if (const auto chambering_animation = logicals.find(stance.chambering)) {
			if (const auto gun = cosm[wielded_items[0]].template find<components::gun>()) {
				const auto frame = ::find_chambering_frame(*gun, *chambering_animation);

				if (frame) {
					return result_t::chambering(*frame);
				}
			}
		}
	}

	if (const auto carry_animation = logicals.find(stance.carry)) {
		return result_t::carry(::get_frame_and_flip(anim_state, *carry_animation));
	}

	return result_t::none();
}

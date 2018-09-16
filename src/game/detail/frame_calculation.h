#pragma once
#include "game/components/gun_component.h"
#include "game/components/movement_component.h"
#include "game/assets/animation_math.h"
#include "game/detail/inventory/weapon_reloading.hpp"

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
	const components::gun& gun,
	T& anim, 
	const cosmos_clock& clk
) {
	const auto animation_time_ms = (clk.now - gun.when_last_fired).in_milliseconds(clk.dt);
	return calc_current_frame(anim, animation_time_ms);
}

template <class T, class C>
frame_type_t<T>* find_shoot_frame(
	const components::gun& gun,
	T& anim, 
	const C& cosm
) {
	return ::find_shoot_frame(gun, anim, cosm.get_clock());
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

template <class T>
struct stance_frame_usage {
	const frame_type_t<T>* const frame;
	flip_flags flip;
	const bool is_shooting;

	static auto none() {
		return stance_frame_usage<T> { nullptr, flip_flags(), false };
	}

	static auto carry(const frame_and_flip<T>& f) {
		return stance_frame_usage<T> { std::addressof(f.frame), f.flip, false };
	}

	static auto grip_to_mag(const frame_type_t<T>& f) {
		return stance_frame_usage<T> { std::addressof(f), flip_flags(), false };
	}

	static auto pocket_to_mag(const frame_type_t<T>& f) {
		return stance_frame_usage<T> { std::addressof(f), flip_flags(), false };
	}

	static auto shoot(const frame_type_t<T>& f) {
		return stance_frame_usage<T> { std::addressof(f), flip_flags(), true };
	}

	static auto chambering(const frame_type_t<T>& f) {
		return stance_frame_usage<T> { std::addressof(f), flip_flags(), true };
	}

	static auto shoot_flipped(const frame_type_t<T>& f) {
		return stance_frame_usage<T> { std::addressof(f), flip_flags::make_vertically(), true };
	}

	explicit operator bool() const {
		return frame != nullptr;
	}

	auto get_with_flip() const {
		return frame_and_flip<T>{ *frame, flip };
	}
};

template <class C, class T>
auto calc_stance_usage(
	const C& cosm,
	const stance_animations& stance,
	const movement_animation_state& anim_state, 
	const T& wielded_items
) {
	using result_t = stance_frame_usage<const torso_animation>;

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
					return (gtm ? result_t::grip_to_mag : result_t::pocket_to_mag)(*found_frame);
				}
			}
		}

		if (const auto shoot_animation = logicals.find(stance.shoot)) {
			/* Determine whether we want a carrying or a shooting animation */
			if (const auto gun = cosm[wielded_items[0]].template find<components::gun>()) {
				const auto frame = ::find_shoot_frame(*gun, *shoot_animation, cosm);
				const auto second_frame = [n, &cosm, shoot_animation, &wielded_items]() -> decltype(frame) {
					if (n == 2) {
						if (const auto second_gun = cosm[wielded_items[1]].template find<components::gun>()) {
							return ::find_shoot_frame(*second_gun, *shoot_animation, cosm);
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
	}

	if (n == 1) {
		if (const auto chambering_animation = logicals.find(stance.get_chambering())) {
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

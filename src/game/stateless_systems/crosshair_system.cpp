#include "crosshair_system.h"
#include "game/cosmos/cosmos.h"

#include "game/messages/intent_message.h"
#include "game/messages/motion_message.h"

#include "game/components/sprite_component.h"

#include "game/components/crosshair_component.h"
#include "game/components/transform_component.h"
#include "game/messages/intent_message.h"

#include "game/cosmos/entity_handle.h"
#include "game/cosmos/logic_step.h"
#include "game/cosmos/data_living_one_step.h"
#include "game/cosmos/for_each_entity.h"
#include "game/modes/detail/fog_of_war_settings.h"
#include "augs/math/math.h"

void crosshair_system::handle_crosshair_intents(const logic_step step) {
	auto& cosm = step.get_cosmos();

	const auto& events = step.get_queue<messages::intent_message>();

	for (const auto& it : events) {
		const auto subject = cosm[it.subject];
		
		if (const auto crosshair = subject.find_crosshair()) {
			if (it.intent == game_intent_type::SWITCH_CAMERA_MODE && it.was_pressed()) {
				auto& mode = crosshair->orbit_mode;

				if (mode == crosshair_orbit_type::LOOK) {
					mode = crosshair_orbit_type::ANGLED;
				}
				else {
					mode = crosshair_orbit_type::LOOK;
				}
			}

			if (it.intent == game_intent_type::TOGGLE_ZOOM_OUT && it.was_pressed()) {
				auto& mode = crosshair->zoom_out_mode;

				mode = !mode;

				if (!mode) {
					/*
						When exiting zoom-out,
						clamp crosshair to the default Fog of War size.
					*/

					const auto default_fow_size = fog_of_war_settings().size;

					crosshair->base_offset = ::clamp_with_raycast(
						crosshair->base_offset,
						default_fow_size * 2
					);
				}
			}
		}
	}
}

void crosshair_system::update_base_offsets(const logic_step step) {
	auto& cosm = step.get_cosmos();

	const auto& events = step.get_queue<messages::motion_message>();
	
	for (const auto& motion : events) {
		if (motion.motion == game_motion_type::MOVE_CROSSHAIR) {
			const auto subject = cosm[motion.subject];

			if (const auto crosshair = subject.find_crosshair()) {
				auto& base_offset = crosshair->base_offset;
				base_offset += motion.offset;

				static_assert(std::is_same_v<remove_cref<decltype(base_offset)>, decltype(motion.offset)>);
			}
		}
	}
}

void crosshair_system::integrate_crosshair_recoils(const logic_step step) {
	const auto dt = step.get_delta();
	const auto secs = dt.in_seconds();
	auto& cosm = step.get_cosmos();

	cosm.for_each_having<components::crosshair>(
		[&](const auto subject) {
			auto& crosshair = subject.template get<components::crosshair>();
			auto& crosshair_def = subject.template get<invariants::crosshair>();
			auto& recoil = crosshair.recoil;

			recoil.integrate(secs);
			recoil.damp(secs, crosshair_def.recoil_damping);

			const auto inertia_mult = [&]() {
				const auto& sentience_def = subject.template get<invariants::sentience>();
				auto& sentience = subject.template get<components::sentience>();

				auto& inertia = sentience.rotation_inertia_ms;

				if (augs::is_positive_epsilon(inertia)) {
					inertia -= dt.in_milliseconds();
					inertia = std::max(inertia, 0.f);

					const auto max_intertia = sentience_def.max_inertia_when_rotation_possible;
					const auto inertia_mult = std::max(0.f, 1.f - inertia / max_intertia);

					return inertia_mult;
				}

				return 1.f;
			}();

			recoil.position.damp(secs, vec2::square(60.f));
			recoil.rotation = augs::damp(recoil.rotation, secs, 60.f * inertia_mult);
		}
	);
}
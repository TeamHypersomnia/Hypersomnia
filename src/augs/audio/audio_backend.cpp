#include "augs/audio/audio_backend.h"
#include "augs/audio/sound_source.h"
#include "augs/audio/audio_command.h"
#include "augs/templates/always_false.h"

#if BUILD_OPENAL
#include <AL/al.h>
#include <AL/alc.h>
#include <AL/alext.h>
#include <AL/efx.h>
#endif

/* A shortcut which will be heavily used from now on */

#if BUILD_OPENAL
template <class A, class B>
constexpr bool same = std::is_same_v<A, B>;
#endif

namespace augs {
	audio_backend::audio_backend() {
#if BUILD_OPENAL
		alGenFilters(1, &lowpass_filter_id);
		alFilteri(lowpass_filter_id, AL_FILTER_TYPE, AL_FILTER_LOWPASS);
#endif
	}

	audio_backend::~audio_backend() {
#if BUILD_OPENAL
		alDeleteFilters(1, &lowpass_filter_id);
#endif
	}

	void audio_backend::perform(
		const audio_command* const c, 
		const std::size_t n
	) {
#if BUILD_OPENAL
		for (std::size_t i = 0; i < n; ++i) {
			const auto& cmd = c[i];

			auto command_handler = [&](const auto& t) {
				using C = remove_cref<decltype(t)>;

				if constexpr(same<C, update_listener_properties>) {
					const auto& si = t.si;
					
					set_listener_position(si, t.position);
					set_listener_velocity(si, t.velocity);

					const auto& orientation = t.orientation;
					set_listener_orientation({ 0.f, -1.f, 0.f, orientation.x, 0.f, orientation.y });
				}
				else if constexpr(same<C, update_multiple_properties>) {
					const auto& source = source_pool[t.proxy_id];
					const auto& si = t.si;

					source.set_position(si, t.position);

					if (t.is_direct_listener) {
						source.set_relative_and_zero_vel_pos();
						source.set_air_absorption_factor(t.air_absorption_factor);
					}
					else {
						source.set_relative(false);
						source.set_position(si, t.position);
						source.set_air_absorption_factor(0.f);
					}

					source.set_spatialize(!t.is_direct_listener);
					source.set_direct_channels(t.is_direct_listener);

					if (t.set_velocity) {
						source.set_velocity(si, t.velocity);
					}

					const bool is_linear = 
						t.model == augs::distance_model::LINEAR_DISTANCE
						|| t.model == augs::distance_model::LINEAR_DISTANCE_CLAMPED
					;

					source.set_gain(t.gain);
					source.set_pitch(t.pitch);
					source.set_doppler_factor(t.doppler_factor);
					source.set_reference_distance(si, t.reference_distance);

					if (is_linear) {
						source.set_max_distance(si, t.max_distance);
					}

					source.set_distance_model(t.model);
					source.set_looping(t.looping);

					if (t.lowpass_gainhf >= 0.f) {
						alFilterf(lowpass_filter_id, AL_LOWPASS_GAINHF, t.lowpass_gainhf);
						alSourcei(source.get_id(), AL_DIRECT_FILTER, lowpass_filter_id);
					}
					else {
						alSourcei(source.get_id(), AL_DIRECT_FILTER, AL_FILTER_NULL);
					}
				}
				else if constexpr(same<C, update_flash_noise>) {
					auto& source = flash_noise_source;

					if (!source.is_playing()) {
						source.bind_buffer(*t.buffer, 0);
						source.set_spatialize(false);
						source.set_direct_channels(true);
						source.set_relative_and_zero_vel_pos();
						source.set_looping(true);
						source.play();
					}

					source.set_gain(t.gain);
				}
				else if constexpr(same<C, reseek_to_sync_if_needed>) {
					const auto& source = source_pool[t.proxy_id];
					const auto actual_secs = source.get_time_in_seconds();

					if (std::abs(t.expected_secs - actual_secs) > t.max_divergence) {
						source.seek_to(t.expected_secs);
					}
				}
				else if constexpr(same<C, bind_sound_buffer>) {
					auto& source = source_pool[t.proxy_id];

					source.bind_buffer(*t.buffer, t.variation_index);
				}
				else if constexpr(same<C, source_no_arg_command>) {
					using A = source_no_arg_command_type;

					auto& source = source_pool[t.proxy_id];

					switch (t.type) {
						case A::PLAY:
							source.play();
							break;
						case A::STOP:
							source.stop();
							break;

						default:
							break;
					}
				}
				else if constexpr(same<C, source1f_command>) {
					using A = source1f_command_type;

					const auto& source = source_pool[t.proxy_id];

					switch (t.type) {
						case A::GAIN:
							source.set_gain(t.v);
						break;
						case A::PITCH:
							source.set_pitch(t.v);
						break;

						default:
							break;
					}
				}
				else {
					static_assert(always_false_v<C>, "Unimplemented command type!");
				}
			};

			std::visit(command_handler, cmd.payload);
		}
#else
		(void)c;
		(void)n;
#endif
	}
}

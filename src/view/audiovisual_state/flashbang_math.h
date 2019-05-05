#pragma once

template <class E>
float get_flash_audio_mult(const E& listener) {
	if (const auto sentience = listener.template find<components::sentience>()) {
		const auto secs = sentience->audio_flash_secs;

		if (secs <= 0.f) {
			return 0.f;
		}

		const auto easing_secs = listener.template get<invariants::sentience>().flash_audio_easing_secs;

		const auto mult = std::min(secs, easing_secs) / easing_secs;

		return mult;
	}

	return 0.f;
}

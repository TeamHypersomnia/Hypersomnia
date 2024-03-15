#pragma once

enum class sound_processing_frequency {
	// GEN INTROSPECTOR enum class sound_processing_frequency
	EVERY_SINGLE_FRAME,
	EVERY_SIMULATION_STEP,
	PERIODIC,

	COUNT
	// END GEN INTROSPECTOR
};

enum class listener_position_reference {
	// GEN INTROSPECTOR enum class listener_position_reference
	SCREEN_CENTER,
	CHARACTER_POSITION,
	COUNT
	// END GEN INTROSPECTOR
};

struct sound_system_settings {
	// GEN INTROSPECTOR struct sound_system_settings
	bool allow_sounds_without_character_listener = true;

	float sync_sounds_longer_than_secs = 5.f;
	float max_divergence_before_sync_secs = 1.f;
	float treat_as_music_sounds_longer_than_secs = 4.f;
	float max_speed_for_doppler_calculation = 4000.f;

	float missile_impact_sound_cooldown_duration = 60.f;
	int missile_impact_occurences_before_cooldown = 1; 

	listener_position_reference listener_reference = listener_position_reference::CHARACTER_POSITION;
	bool set_listener_orientation_to_character_orientation = false;
	int max_simultaneous_bullet_trace_sounds = 6;
	float gain_threshold_for_bullet_trace_sounds = 0.012f;
	int max_short_sounds = 64;

	sound_processing_frequency processing_frequency = sound_processing_frequency::EVERY_SIMULATION_STEP;
	int custom_processing_frequency = 10;
	// END GEN INTROSPECTOR
};

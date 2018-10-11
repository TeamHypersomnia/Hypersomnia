#pragma once

struct cosmic_entropy_recording_options {
	// GEN INTROSPECTOR struct entropy_recording_options
	bool overwrite_intents = true;
	bool overwrite_motions = true;
	bool overwrite_rest = true;
	// END GEN INTROSPECTOR

	void neg() {
		auto f = [&](auto& ff) {
			ff = !ff;
		};

		f(overwrite_intents);
		f(overwrite_motions);
		f(overwrite_rest);
	}
};

struct mode_entropy_recording_options {
	// GEN INTROSPECTOR struct mode_entropy_recording_options
	bool overwrite = true;
	// END GEN INTROSPECTOR

	void neg() {
		auto f = [&](auto& ff) {
			ff = !ff;
		};

		f(overwrite);
	}
};

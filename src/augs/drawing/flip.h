#pragma once

struct flip_flags {
	static flip_flags make_horizontally() {
		flip_flags f;
		f.horizontally = true;
		return f;
	}

	static flip_flags make_vertically() {
		flip_flags f;
		f.vertically = true;
		return f;
	}

	// GEN INTROSPECTOR struct flip_flags
	bool horizontally = false;
	bool vertically = false;
	// END GEN INTROSPECTOR
};
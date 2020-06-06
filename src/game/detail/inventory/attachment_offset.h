#pragma once

struct attachment_offset {
	transformr offset;
	bool flip_geometry = false;

	attachment_offset operator*(const attachment_offset& b) const {
		auto next_offset = b.offset;

		if (flip_geometry) {
			next_offset.flip_vertically();
		}

		return { offset * next_offset, b.flip_geometry ? !flip_geometry : flip_geometry };
	}
};


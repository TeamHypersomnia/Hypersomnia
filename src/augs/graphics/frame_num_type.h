#pragma once

namespace augs {
	using frame_num_type = std::size_t;

	inline bool has_frame_completed(const frame_num_type current_frame, const frame_num_type n) {
		return current_frame > n + 1;
	}

	inline bool has_completed(const frame_num_type current_frame, std::optional<frame_num_type> n) {
		return !n || has_frame_completed(current_frame, *n);
	}
}

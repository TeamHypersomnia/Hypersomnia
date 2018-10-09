#pragma once

namespace augs {
	struct assignment_detector {
		int count = 0;

		assignment_detector() : count(1) {}

		assignment_detector(assignment_detector&&) {
			++count;
		}

		assignment_detector(const assignment_detector&) {
			++count;
		}

		assignment_detector& operator=(assignment_detector&&) {
			++count;
			return *this;
		}

		assignment_detector& operator=(const assignment_detector&) {
			++count;
			return *this;
		}
	};
}

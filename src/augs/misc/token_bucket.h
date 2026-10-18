#pragma once
#include <algorithm>

namespace augs {
	/*
		A rate limit: per_sec events a second on average, at most burst at once.
		A fresh bucket starts full.
	*/

	struct token_bucket {
		double tokens = 0.0;
		double when_last = -1.0;

		bool take(const double now, const double per_sec, const double burst) {
			if (when_last < 0.0) {
				tokens = burst;
			}
			else {
				tokens = std::min(burst, tokens + (now - when_last) * per_sec);
			}

			when_last = now;

			if (tokens < 1.0) {
				return false;
			}

			tokens -= 1.0;
			return true;
		}
	};
}

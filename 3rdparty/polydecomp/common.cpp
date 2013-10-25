#include "common.h"

namespace poly_decomposition {
	Scalar min(const Scalar &a, const Scalar &b) {
		return a < b ? a : b;
	}

	bool eq(const Scalar &a, const Scalar &b) {
		return abs(a - b) <= 1e-8;
	}

	int wrap(const int &a, const int &b) {
		return a < 0 ? a % b + b : a % b;
	}

	Scalar srand(const Scalar &min = 0, const Scalar &max = 1) {
		return rand() / (Scalar) RAND_MAX * (max - min) + min;
	}
}

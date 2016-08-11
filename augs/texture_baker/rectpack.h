#pragma once
#include <vector>
#if INTEGRATE_RECTPACK_WITH_AUGMENTATIONS
#include "augs/math/rects.h"
#else
namespace augs {
	namespace rects {

	}
}
#endif

namespace augs {
	namespace packing {
		struct bin {
			rects::wh<int> size;
			std::vector<rects::xywhf<int>*> rects;
		};

		rects::wh<int> _rect2D(rects::xywhf<int>* const * v, int n, int max_s, std::vector<rects::xywhf<int>*>* succ = 0, std::vector<rects::xywhf<int>*>* unsucc = 0);

		/*
		0 - succesful
		1 - couldn't pack into max_bins/one bin
		2 - rect larger than bin exists
		*/

		int rect2D(rects::xywhf<int>* const * v, int n, int max_side, bin& out_bin);
		int rect2D(rects::xywhf<int>* const * v, int n, int max_side, std::vector<bin>& bins);
		int rect2D(rects::xywhf<int>* const * v, int n, int max_side, std::vector<bin>& bins, int max_bins); 
	}
}
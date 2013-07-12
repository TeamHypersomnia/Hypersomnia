#pragma once
#include <vector>
#include "../rects/rects.h"

namespace augmentations {
	namespace packing {
		struct bin {
			rects::wh size;
			std::vector<rects::xywhf*> rects;
		};

		rects::wh _rect2D(rects::xywhf* const * v, int n, int max_s, std::vector<rects::xywhf*>* succ = 0, std::vector<rects::xywhf*>* unsucc = 0);

		/*
		0 - succesful
		1 - couldn't pack into max_bins/one bin
		2 - rect larger than bin exists
		*/

		int rect2D(rects::xywhf* const * v, int n, int max_side, bin& out_bin);
		int rect2D(rects::xywhf* const * v, int n, int max_side, std::vector<bin>& bins);
		int rect2D(rects::xywhf* const * v, int n, int max_side, std::vector<bin>& bins, int max_bins); 
	}
}
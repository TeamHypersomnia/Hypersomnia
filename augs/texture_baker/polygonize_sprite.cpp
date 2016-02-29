#include "image.h"
#include "log.h"

namespace augs {
	std::vector<vec2i> image::get_polygonized(int max_vertices) const {
		std::vector<vec2i> sample;
		sample.resize(4);

		rgba p = pixel(0, 0);
		LOG("%x, %x, %x, %x", p.r, p.g, p.b, p.a);

		sample[0] = vec2i(0, 0);
		sample[1] = vec2i(size.w, 0);
		sample[2] = vec2i(size.w, size.h);
		sample[3] = vec2i(0, size.h);
		return sample;
	}
}


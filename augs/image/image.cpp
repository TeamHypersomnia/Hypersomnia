#include "augs/image/image.h"
#include "3rdparty/lodepng/lodepng.h"
#include "augs/ensure.h"
#include "augs/filesystem/directory.h"

template<class C>
static void Line(
	int x1, 
	int y1, 
	int x2, 
	int y2, 
	C callback
) {
	// Bresenham's line algorithm
	const bool steep = (std::abs(y2 - y1) > std::abs(x2 - x1));
	if (steep)
	{
		std::swap(x1, y1);
		std::swap(x2, y2);
	}

	if (x1 > x2)
	{
		std::swap(x1, x2);
		std::swap(y1, y2);
	}

	const int dx = x2 - x1;
	const int dy = std::abs(y2 - y1);

	float error = dx / 2.0f;
	const int ystep = (y1 < y2) ? 1 : -1;
	int y = (int)y1;

	const int maxX = (int)x2;

	for (int x = (int)x1; x<=maxX; x++)
	{
		if (steep)
		{
			callback(y, x);
		}
		else
		{
			callback(x, y);
		}

		error -= dy;
		if (error < 0)
		{
			y += ystep;
			error += dx;
		}
	}
}

namespace augs {
	void image::create(const vec2u new_size) {
		size = new_size;
		v.resize(new_size.area(), rgba(0, 0, 0, 0));
	}

	void image::paint_circle_midpoint(
		const unsigned radius, 
		const unsigned border_width, 
		const rgba filling, 
		const bool scale_alpha, 
		const bool constrain_angle, 
		const vec2 angle_start, 
		const vec2 angle_end
	) {
		const auto side = radius * 2 + 1;

		image new_surface;
		auto& surface = v.empty() ? *this : new_surface;
		
		surface.create({ std::max(size.x, side), std::max(size.y, side) });

		ensure(size.x >= side);
		ensure(size.y >= side);

		const auto pp = [&](
			const unsigned x, 
			const unsigned y
		) {
			const auto x_center = static_cast<signed>(x - size.x / 2);
			const auto y_center = static_cast<signed>(y - size.y / 2);

			const auto angle = vec2i(x_center, y_center);// .degrees();

			if (!constrain_angle || (angle_start.cross(angle) >= 0.f && angle_end.cross(angle) <= 0.f)) {
				const auto col = filling;
				//if (scale_alpha)
				//	col.a = (angle - angle_start) / (angle_end - angle_start) * 255;

				surface.pixel({ x, y } ) = col;
			}
		};

		for (unsigned i = 0; i < border_width; ++i) {
			int x = radius - i;
			int y = 0;
			
			int decisionOver2 = 1 - x;   // Decision criterion divided by 2 evaluated at x=r, y=0
			const auto x0 = size.x / 2;
			const auto y0 = size.y / 2;

			while (y <= x)
			{
				pp(x + x0, y + y0); // Octant 1
				pp(y + x0, x + y0); // Octant 2
				pp(-x + x0, y + y0); // Octant 4
				pp(-y + x0, x + y0); // Octant 3
				pp(-x + x0, -y + y0); // Octant 5
				pp(-y + x0, -x + y0); // Octant 6
				pp(x + x0, -y + y0); // Octant 7
				pp(y + x0, -x + y0); // Octant 8
				y++;

				if (decisionOver2 <= 0) {
					decisionOver2 += 2 * y + 1;   // Change in decision criterion for y -> y+1
				}
				else {
					decisionOver2 += 2 * (y - (--x)) + 1;   // Change for y -> y+1, x -> x-1
				}
			}
		}

		if (border_width > 1) {
			for (unsigned y = 0; y < surface.size.x; ++y) {
				for (unsigned x = 0; x < surface.size.x; ++x) {
					if (x > 0 && y > 0 && x < surface.size.x - 1 && y < surface.size.y - 1) {
						if (surface.pixel({ x, y }).a == 0 &&
							surface.pixel({x + 1, y}).a > 0 &&
							surface.pixel({x - 1, y}).a > 0 &&
							surface.pixel({x, y + 1}).a > 0 &&
							surface.pixel({x, y - 1}).a > 0)
							pp(x, y);
					}
				}
			}
		}

		if (&surface != this) {
			blit(
				surface,
				{ 0u, 0u },
				false,
				true
			);
		}
	}

	void image::paint_circle(
		const unsigned radius, 
		const unsigned border_width, 
		const rgba filling, 
		const bool scale_alpha
	) {
		const auto side = radius * 2 + 1;

		if (v.empty()) {
			create({ side, side });
		}
		else {
			ensure(size.x >= side);
			ensure(size.y >= side);
		}

		if (scale_alpha) {
			for (unsigned y = 0; y < size.y; ++y) {
				for (unsigned x = 0; x < size.x; ++x) {
					const auto x_center = static_cast<signed>(x - size.x / 2);
					const auto y_center = static_cast<signed>(y - size.y / 2);

					const auto angle = vec2i(x_center, y_center).degrees();

					if (angle >= -45 && angle <= 45 &&
						x_center*x_center + y_center*y_center <= radius*radius
						&&
						x_center*x_center + y_center*y_center >= (radius - border_width)*(radius - border_width)
						) {
						pixel({x, y}) = filling;
						pixel({x, y}).a = static_cast<rgba_channel>((angle + 45) / 90.f * 255);
					}
				}
			}
		}
		else {
			for (unsigned y = 0; y < size.y; ++y) {
				for (unsigned x = 0; x < size.x; ++x) {
					const auto x_center = static_cast<signed>(x - size.x / 2);
					const auto y_center = static_cast<signed>(y - size.y / 2);

					if (
						x_center*x_center + y_center*y_center <= radius*radius
						&&
						x_center*x_center + y_center*y_center >= (radius - border_width)*(radius - border_width)
						) {
						pixel({ x, y }) = filling;
					}
				}
			}
		}
		//pixel(side / 2, 0) = rgba(0, 0, 0, 0);
		//pixel(side / 2, side - 1) = rgba(0, 0, 0, 0);
		//pixel(0, side / 2) = rgba(0, 0, 0, 0);
		//pixel(side - 1, side / 2) = rgba(0, 0, 0, 0);
	}

	void image::paint_filled_circle(
		const unsigned radius, 
		const rgba filling
	) {
		const auto side = radius * 2 + 1;

		if (v.empty()) {
			create({ side, side });
		}
		else {
			ensure(size.x >= side);
			ensure(size.y >= side);
		}

		for (unsigned y = 0; y < size.y; ++y) {
			for (unsigned x = 0; x < size.x; ++x) {
				const auto x_center = static_cast<signed>(x - size.x / 2);
				const auto y_center = static_cast<signed>(y - size.y / 2);

				if (x_center*x_center + y_center*y_center <= radius*radius) {
					pixel({ x, y }) = filling;
				}
			}
		}

		//pixel(side / 2, 0) = rgba(0, 0, 0, 0);
		//pixel(side / 2, side -1) = rgba(0, 0, 0, 0);
		//pixel(0, side /2) = rgba(0, 0, 0, 0);
		//pixel(side -1, side /2) = rgba(0, 0, 0, 0);
	}

	void image::paint_line(
		const vec2u from, 
		const vec2u to, 
		const rgba filling
	) {
		Line(from.x, from.y, to.x, to.y, [&](const int x, const int y) {
			const auto pos = vec2u(x, y);

			ensure(in_bounds(pos));
			pixel(pos) = filling;
		});
	}
	
	bool image::in_bounds(const vec2u v) const {
		return v.x < size.x && v.y < size.y;
	}

	bool image::from_file(
		const std::string& filename
	) {
		unsigned width;
		unsigned height;

		if (lodepng::decode(*reinterpret_cast<std::vector<unsigned char>*>(&v), width, height, filename)) {
			LOG("Failed to open %x! Ensure that the file exists and has correct format.", filename);
			ensure(false);
			return false;
		}

		size.x = width;
		size.y = height;

		return true;
	}

	void image::swap_red_and_blue() {
		for (auto& p : v) {
			std::swap(p.r, p.b);
		}
	}

	void image::save(const std::string& filename) const {
		augs::create_directories(filename);

		if (lodepng::encode(filename, *reinterpret_cast<const std::vector<unsigned char>*>(&v), size.x, size.y)) {
			LOG("Could not encode %x! Ensure that the target directory exists.", filename);
		}
	}

	void image::fill(const rgba col) {
		for (auto& p : v) {
			p = col;
		}
	}

	void image::create_from(
		const unsigned char* const ptr, 
		const unsigned channels,
		const unsigned pitch, 
		const vec2u size
	) {
		create(size);

		const int wbytes = size.x*channels;

		if (channels == 1) {
			for (unsigned j = 0; j < size.y; ++j) {
				for (unsigned i = 0; i < size.x; ++i) {
					pixel({ i, j }) = { 255, 255, 255, ptr[pitch*j+i] };
				}
			}
		}
		else {
			ensure(false);
		}
	}

	void image::blit(
		const image& source_image, 
		const vec2u dst,
		const bool flip_source,
		const bool add_rgba_values
	) {
		const auto source_size = source_image.get_size();

		if (!add_rgba_values) {
			if (flip_source) {
				for (auto y = 0u; y < source_size.y; ++y) {
					for (auto x = 0u; x < source_size.x; ++x) {
						pixel(dst + vec2u{ y, x }) = source_image.pixel(vec2u{ x, y });
					}
				}
			}
			else {
				for (auto y = 0u; y < source_size.y; ++y) {
					for (auto x = 0u; x < source_size.x; ++x) {
						pixel(dst + vec2u{ x, y }) = source_image.pixel(vec2u{ x, y });
					}
				}
			}
		}
		else {
			if (flip_source) {
				for (auto y = 0u; y < source_size.y; ++y) {
					for (auto x = 0u; x < source_size.x; ++x) {
						pixel(dst + vec2u{ y, x }) += source_image.pixel(vec2u{ x, y });
					}
				}
			}
			else {
				for (auto y = 0u; y < source_size.y; ++y) {
					for (auto x = 0u; x < source_size.x; ++x) {
						pixel(dst + vec2u{ x, y }) += source_image.pixel(vec2u{ x, y });
					}
				}
			}
		}
	}

	const rgba_channel* image::get_data() const {
		return reinterpret_cast<const rgba_channel*>(v.data());
	}

	rgba& image::pixel(const vec2u pos) {
		return v[pos.y * size.x + pos.x];
	}

	const rgba& image::pixel(const vec2u pos) const {
		return v[pos.y * size.x + pos.x];
	}

	vec2u image::get_size() const {
		return size;
	}

	unsigned image::get_rows() const {
		return size.y;
	}

	unsigned image::get_columns() const {
		return size.x;
	}

	image image::get_desaturated() const {
		image desaturated;
		desaturated.create(size);

		for (unsigned y = 0; y < size.y; ++y) {
			for (unsigned x = 0; x < size.x; ++x) {
				desaturated.pixel({ x, y }) = pixel({ x, y }).get_desaturated();
			}
		}

		return std::move(desaturated);
	}

	void image::destroy() {
		v.clear();
		v.shrink_to_fit();
		size.reset();
	}
}




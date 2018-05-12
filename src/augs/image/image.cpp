#include <cstring>

#include "3rdparty/lodepng/lodepng.h"
#include "augs/ensure.h"

#include "augs/image/image.h"
#include "augs/filesystem/directory.h"
#include "augs/filesystem/file.h"
#include "augs/readwrite/byte_readwrite.h"

#if PLATFORM_UNIX
#include <arpa/inet.h>
#else
#include <winsock.h>
#endif

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
	static void throw_if_zero_size(const path_type& path, const vec2u size) {
		if (!size.x || !size.y) {
			throw image_loading_error("Failed to load image %x:\nWidth or height is zero!", path);
		}
	}

	vec2u image::get_size(const path_type& file_path) {
		try {
			const auto extension = file_path.extension();
			auto in = with_exceptions<std::ifstream>(file_path);

			if (extension == ".png") {
				unsigned int width, height;

				in.seekg(16);
				in.read((char*)&width, 4);
				in.read((char*)&height, 4);

				width = ntohl(width);
				height = ntohl(height);

				return { width, height };
			}
			else if (extension == ".bin") {
				vec2u s;
				augs::read_bytes(in, s);
				return s;
			}
			else {
				throw image_loading_error(
					"Failed to read size of %x:\nUnknown image extension: %x!", file_path, extension
				);
			}
		}
		catch (image_loading_error err) {
			throw;
		}
		catch (std::exception err) {
			throw image_loading_error("Failed to read size of %x:\n%x", file_path, err.what());
		}
	}

	image::image(const vec2u new_size) {
		resize_fill(new_size);
	}
	
	image::image(
		const unsigned char* const ptr, 
		const unsigned channels,
		const unsigned pitch, 
		const vec2u new_size
	) {
		resize_no_fill(new_size);

		if (channels == 1) {
			for (unsigned j = 0; j < new_size.y; ++j) {
				for (unsigned i = 0; i < new_size.x; ++i) {
					pixel({ i, j }) = { 255, 255, 255, ptr[pitch * j + i] };
				}
			}
		}
		else if (channels == 4 && pitch == 0) {
			std::memcpy(v.data(), ptr, new_size.area() * 4);
		}
		else {
			ensure(false);
		}
	}

	image::image(const path_type& path) {
		from_file(path);
	}
	
	void image::from_file(const path_type& path) {
		const auto extension = path.extension();

		if (extension == "") {
			const auto resolved_path = augs::switch_path(
				augs::path_type(path) += ".png",
				augs::path_type(path) += ".bin"
			);

			from_file(resolved_path);
		}
		else {
			if (extension == ".png") {
				from_png(path);
			}
			else if (extension == ".bin") {
				from_binary_file(path);
			}
			else {
				throw image_loading_error(
					"Failed to load image %x:\nUnknown image extension: %x!", path, extension
				);
			}
		}

		throw_if_zero_size(path, size);
	}

	void image::from_binary_file(const path_type& path) try {
		auto in = with_exceptions<std::ifstream>();
		in.open(path, std::ios::in | std::ios::binary);

		augs::read_bytes(in, size);
		augs::read_bytes(in, v);

		throw_if_zero_size(path, size);
	}
	catch (const augs::file_open_error& err) {
		throw image_loading_error(
			"Failed to load image %x:\n%x", path, err.what()
		);
	}


	void image::from_png(
		const std::vector<std::byte>& from, 
		const path_type& reported_path
	) {
		v.clear();

		unsigned width;
		unsigned height;
		using lodepng_vec = std::vector<unsigned char>;

		if (const auto lodepng_result =
			lodepng::decode(
				reinterpret_cast<lodepng_vec&>(v), 
				width, 
				height, 
				reinterpret_cast<const lodepng_vec&>(from)
			)
		) {
			throw image_loading_error(
				"Failed to load image %x (earlier loaded into memory):\nlodepng returned %x", reported_path, lodepng_result
			);
		}

		size.x = width;
		size.y = height;

		throw_if_zero_size(reported_path, size);
	}

	void image::from_png(const path_type& path) {
		v.clear();

		unsigned width;
		unsigned height;

		if (const auto lodepng_result =
			lodepng::decode(*reinterpret_cast<std::vector<unsigned char>*>(&v), width, height, path.string())
		) {
			throw image_loading_error(
				"Failed to load image %x:\nlodepng returned %x", path, lodepng_result
			);
		}

		size.x = width;
		size.y = height;

		throw_if_zero_size(path, size);
	}

	void image::execute(const paint_command_variant& in) {
		std::visit([&](const auto& resolved) {
			execute(resolved);
		}, in);
	}

	void image::execute(
		const paint_circle_midpoint_command& in
	) {
		const auto side = in.radius * 2 + 1;

		image new_surface;
		auto& surface = v.empty() ? *this : new_surface;
		
		surface = augs::image(vec2u { std::max(size.x, side), std::max(size.y, side) });

		ensure(size.x >= side);
		ensure(size.y >= side);

		const auto angle_start = vec2::from_degrees(in.angle_start);
		const auto angle_end = vec2::from_degrees(in.angle_end);

		const auto pp = [&](
			const unsigned x, 
			const unsigned y
		) {
			const auto x_center = static_cast<signed>(x - size.x / 2);
			const auto y_center = static_cast<signed>(y - size.y / 2);

			const auto angle = vec2i(x_center, y_center);// .degrees();

			if (!in.constrain_angle || (angle_start.cross(vec2(angle)) >= 0.f && angle_end.cross(vec2(angle)) <= 0.f)) {
				const auto col = in.filling;
				//if (scale_alpha)
				//	col.a = (angle - angle_start) / (angle_end - angle_start) * 255;

				surface.pixel({ x, y } ) = col;
			}
		};

		for (unsigned i = 0; i < in.border_width; ++i) {
			int x = in.radius - i;
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

		if (in.border_width > 1) {
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

	void image::execute(
		const paint_circle_filled_command& in
	) {
		const auto side = in.radius * 2 + 1;

		if (v.empty()) {
			resize_fill({ side, side });
		}
		else {
			ensure(size.x >= side);
			ensure(size.y >= side);
		}

		for (unsigned y = 0; y < size.y; ++y) {
			for (unsigned x = 0; x < size.x; ++x) {
				const auto x_center = static_cast<signed>(x - size.x / 2);
				const auto y_center = static_cast<signed>(y - size.y / 2);

				if (x_center*x_center + y_center*y_center <= static_cast<int>(in.radius*in.radius)) {
					pixel({ x, y }) = in.filling;
				}
			}
		}

		//pixel(side / 2, 0) = rgba(0, 0, 0, 0);
		//pixel(side / 2, side -1) = rgba(0, 0, 0, 0);
		//pixel(0, side /2) = rgba(0, 0, 0, 0);
		//pixel(side -1, side /2) = rgba(0, 0, 0, 0);
	}

	void image::execute(
		const paint_line_command& in
	) {
		Line(in.from.x, in.from.y, in.to.x, in.to.y, [&](const int x, const int y) {
			const auto pos = vec2u(x, y);

			ensure(in_bounds(pos));
			pixel(pos) = in.filling;
		});
	}
	
	void image::swap_red_and_blue() {
		for (auto& p : v) {
			std::swap(p.r, p.b);
		}
	}
	
	void image::save(const path_type& path) const {
		const auto extension = path.extension();

		if (extension == ".png") {
			save_as_png(path);
		}
		else if (extension == ".bin") {
			save_as_binary_file(path);
		}
		else {
			LOG("Warning: %x has unknown extension: %x! Saving as binary.", path, extension);
			save_as_binary_file(path);
		}
	}

	void image::save_as_png(const path_type& path) const {
		augs::create_directories_for(path);

		if (
			const auto lodepng_result = 
			lodepng::encode(path.string(), *reinterpret_cast<const std::vector<unsigned char>*>(&v), size.x, size.y)
		) {
			LOG("Failed to save %x: lodepng returned %x. Ensure that the target directory exists.", path, lodepng_result);
		}
	}

	void image::save_as_binary_file(const path_type& path) const {
		augs::create_directories_for(path);

		std::ofstream out(path, std::ios::out | std::ios::binary);
		augs::write_bytes(out, size);
		augs::write_bytes(out, v);
	}

	void image::fill(const rgba col) {
		for (auto& p : v) {
			p = col;
		}
	}

	void image::blit(
		const image& source_image, 
		const vec2u dst,
		const bool flip_source,
		const bool additive
	) {
		const auto source_size = source_image.get_size();

		if (!additive) {
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

	image_view::image_view(rgba* v, vec2u size) : v(v), size(size) {}
	 

	void image_view::fill(const rgba fill_color) {
		for (auto y = 0u; y < size.y; ++y) {
			for (auto x = 0u; x < size.x; ++x) {
				pixel(vec2u{ y, x }) = fill_color;
			}
		}
	}

	void image_view::blit(
		const image& source_image, 
		const vec2u dst,
		const bool flip_source,
		const bool additive
	) {
		const auto source_size = source_image.get_size();

		if (!additive) {
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

	image& image::desaturate() {
		for (unsigned y = 0; y < size.y; ++y) {
			for (unsigned x = 0; x < size.x; ++x) {
				pixel({ x, y }).desaturate();
			}
		}

		return *this;
	}
}




#pragma once
#include <vector>
#include "augs/math/vec2.h"
#include "augs/graphics/pixel.h"

namespace augs {
	class image {
		std::vector<rgba_channel> v;
		vec2i size;
		int channels;

	public:
		image();
		image(const image&);
		image& operator=(const image&);

		void paint_circle(int radius, int border_width = 1, rgba filling = white, bool scale_alpha = false);
		void paint_circle_midpoint(int radius, int border_width = 1, rgba filling = white, bool scale_alpha = false, bool constrain_angle = false,
			vec2 angle_start = vec2(), vec2 angle_end = vec2());
		void paint_filled_circle(int radius, rgba filling = white);

		void paint_button_with_cuts(int width, int height, int left_bottom_cut_length, int top_right_cut_length, rgba border, rgba filling);

		void paint_line(const vec2i from, const vec2i to, const rgba filling = white);
		bool in_bounds(const vec2i) const;

		void create(int w, int h, int channels);
		bool from_file(const std::string& filename, unsigned channels = 0),
			from_clipboard();

		void swap_red_and_blue();

		void save(const std::string& filename);

		void fill(unsigned char val),
			fill(unsigned char* channel_vals),
			fill(const rgba),
			fill_channel(int channel, unsigned char val);

		void copy(const image&);
		void copy(unsigned char* ptr, int channels, int pitch, const vec2i& size);

		void blit(const image&, int x, int y, const rects::xywhf<int>& src, bool luminance_to_alpha = false, bool add = false);
		void blit_channel(const image&, int x, int y, const rects::xywhf<int>& src, int channel, int src_channel);

		unsigned char* operator()(int x, int y, int channel = 0); // get pixel
		unsigned char* ptr(int x = 0, int y = 0, int channel = 0);
		const unsigned char* ptr(int x = 0, int y = 0, int channel = 0) const;
		unsigned char  pix(int x = 0, int y = 0, int channel = 0) const;

		rgba& pixel(int x, int y);
		const rgba& pixel(int x, int y) const;

		void set_pixel(const vec2i pos, const rgba);
		const rgba& pixel(vec2i pos) const;

		std::vector<vec2i> get_polygonized() const;

		int get_channels() const, get_bytes() const, get_num_pixels() const;
		vec2i get_size() const;

		image get_desaturated() const;

		void destroy();
	};
}
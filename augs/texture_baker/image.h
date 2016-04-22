#pragma once
#include <vector>
#include "../math/vec2.h"
#include "../graphics/pixel.h"

namespace augs {
	class image {
		std::vector<augs::rgba_channel> v;
		rects::wh<int> size;
		int channels;

	public:
		image();
		image(const image&);
		image& operator=(const image&);

		void paint_circle(int radius, int border_width = 1, augs::rgba filling = white, bool scale_alpha = false);
		void paint_circle_midpoint(int radius, int border_width = 1, augs::rgba filling = white, bool constrain_angle = false, 
			float angle_start = -1, float angle_end = -1, bool scale_alpha = false);
		void paint_filled_circle(int radius, augs::rgba filling = white);

		void create(int w, int h, int channels);
		bool from_file(const std::wstring& filename, bool swap_red_and_blue = true, unsigned channels = 0),
			from_clipboard();

		void save(const std::wstring& filename);

		void fill(unsigned char val),
			fill(unsigned char* channel_vals),
			fill_channel(int channel, unsigned char val);

		void copy(const image&);
		void copy(unsigned char* ptr, int channels, int pitch, const rects::wh<int>& size);

		void blit(const image&, int x, int y, const rects::xywhf<int>& src, bool luminance_to_alpha = false, bool add = false);
		void blit_channel(const image&, int x, int y, const rects::xywhf<int>& src, int channel, int src_channel);

		unsigned char* operator()(int x, int y, int channel = 0); // get pixel
		unsigned char* ptr(int x = 0, int y = 0, int channel = 0);
		const unsigned char* ptr(int x = 0, int y = 0, int channel = 0) const;
		unsigned char  pix(int x = 0, int y = 0, int channel = 0) const;

		rgba& pixel(int x, int y);
		const rgba& pixel(int x, int y) const;

		const rgba& pixel(vec2i pos) const;

		std::vector<vec2i> get_polygonized() const;

		int get_channels() const, get_bytes() const, get_num_pixels() const;
		const rects::wh<int>& get_size() const;

		void destroy();
	};
}
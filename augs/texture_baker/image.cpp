#pragma once
#include "image.h"
#include "lodepng.h"
#include "ensure.h"

namespace augs {
	image::image() : size(0, 0), channels(0) {}

	image::image(const image& img) {
		copy(img);
	}

	image& image::operator=(const image& img) {
		copy(img);
		return *this;
	}

	void image::create(int w, int h, int ch) {
		size = vec2i(w, h);
		channels = ch;
		v.resize(get_bytes(), 0);
		//v.shrink_to_fit();
	}

	void image::paint_circle_midpoint(int radius, int border_width, augs::rgba filling, bool scale_alpha, bool constrain_angle, vec2 angle_start, vec2 angle_end) {
		auto side = radius * 2 + 1;

		image new_surface;
		auto& surface = v.empty() ? *this : new_surface;
		
		surface.create(std::max(size.w, side), std::max(size.h, side), 4);

		ensure(size.w >= side);
		ensure(size.h >= side);

		auto pp = [&](int x, int y){
			int x_center = x - size.w / 2;
			int y_center = y - size.h / 2;

			auto angle = vec2(x_center, y_center);// .degrees();

			if (!constrain_angle || (angle_start.cross(angle) >= 0.f && angle_end.cross(angle) <= 0.f)) {
				auto col = filling;
				//if (scale_alpha)
				//	col.a = (angle - angle_start) / (angle_end - angle_start) * 255;

				surface.pixel(x, y) = col;
			}
		};

		for (int i = 0; i < border_width; ++i) {
			int x = radius - i;
			int y = 0;
			int decisionOver2 = 1 - x;   // Decision criterion divided by 2 evaluated at x=r, y=0
			int x0 = size.w / 2;
			int y0 = size.h / 2;

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
				if (decisionOver2 <= 0)
				{
					decisionOver2 += 2 * y + 1;   // Change in decision criterion for y -> y+1
				}
				else
				{
					decisionOver2 += 2 * (y - (--x)) + 1;   // Change for y -> y+1, x -> x-1
				}
			}
		}

		if (border_width > 1) {
			for (int x = 0; x < surface.size.w; ++x) {
				for (int y = 0; y < surface.size.w; ++y) {
					if (x > 0 && y > 0 && x < surface.size.w - 1 && y < surface.size.h - 1) {
						if (surface.pixel(x, y).a == 0 &&
							surface.pixel(x + 1, y).a > 0 &&
							surface.pixel(x - 1, y).a > 0 &&
							surface.pixel(x, y + 1).a > 0 &&
							surface.pixel(x, y - 1).a > 0)
							pp(x, y);
					}
				}
			}
		}

		if (&surface != this) {
			blit(surface, 0, 0, rects::xywhf<int>(0, 0, surface.size.w, surface.size.h, false), false, true);
		}
	}

	void image::paint_circle(int radius, int border_width, augs::rgba filling, bool scale_alpha) {
		auto side = radius * 2 + 1;

		if (v.empty()) {
			create(side, side, 4);
		}
		else {
			ensure(size.w >= side);
			ensure(size.h >= side);
		}

		if (scale_alpha) {
			for (int x = 0; x < size.w; ++x) {
				for (int y = 0; y < size.h; ++y) {
					int x_center = x - size.w / 2;
					int y_center = y - size.h / 2;

					auto angle = vec2(x_center, y_center).degrees();

					if (angle >= -45 && angle <= 45 &&
						x_center*x_center + y_center*y_center <= radius*radius
						&&
						x_center*x_center + y_center*y_center >= (radius - border_width)*(radius - border_width)
						) {
						pixel(x, y) = filling;
						pixel(x, y).a = (angle + 45) / 90.f * 255;
					}
				}
			}
		}
		else {

			for (int x = 0; x < size.w; ++x) {
				for (int y = 0; y < size.h; ++y) {
					int x_center = x - size.w / 2;
					int y_center = y - size.h / 2;

					if (
						x_center*x_center + y_center*y_center <= radius*radius
						&&
						x_center*x_center + y_center*y_center >= (radius - border_width)*(radius - border_width)
						) {
						pixel(x, y) = filling;
					}
				}
			}
		}
		//pixel(side / 2, 0) = rgba(0, 0, 0, 0);
		//pixel(side / 2, side - 1) = rgba(0, 0, 0, 0);
		//pixel(0, side / 2) = rgba(0, 0, 0, 0);
		//pixel(side - 1, side / 2) = rgba(0, 0, 0, 0);
	}

	void image::paint_filled_circle(int radius, augs::rgba filling) {
		auto side = radius * 2 + 1;

		if (v.empty()) {
			create(side, side, 4);
		}
		else {
			ensure(size.w >= side);
			ensure(size.h >= side);
		}

		for (int x = 0; x < size.w; ++x) {
			for (int y = 0; y < size.h; ++y) {
				int x_center = x - size.w / 2;
				int y_center = y - size.h / 2;

				if (x_center*x_center + y_center*y_center <= radius*radius) {
					pixel(x, y) = filling;
				}
			}
		}

		//pixel(side / 2, 0) = rgba(0, 0, 0, 0);
		//pixel(side / 2, side -1) = rgba(0, 0, 0, 0);
		//pixel(0, side /2) = rgba(0, 0, 0, 0);
		//pixel(side -1, side /2) = rgba(0, 0, 0, 0);
	}

	void image::paint_button_with_cuts(int width, int height, int left_bottom_cut_length, int top_right_cut_length, augs::rgba border, augs::rgba filling) {
		create(width, height, 4);

		for (int x = 0; x < size.w; ++x) {
			for (int y = 0; y < size.h; ++y) {
				if (!x || !y || x == size.w - 1 || y == size.h - 1)
					pixel(x, y) = border;
				else
					pixel(x, y) = filling;
			}
		}
	}

	bool image::from_file(const std::wstring& filename, unsigned force_channels) {
		channels = 4;

		std::string lodepngfname(filename.begin(), filename.end());

		unsigned width;
		unsigned height;

		if (lodepng::decode(v, width, height, lodepngfname)) {
			ensure(0);
			return false;
		}

		size.w = width;
		size.h = height;

		return true;
	}

	void image::swap_red_and_blue() {
		for (int i = 0; i < size.area(); ++i)
			std::swap(v[i * 4 + 0], v[i * 4 + 2]);
	}

	void image::save(const std::wstring& filename) {
		std::string lodepngfname(filename.begin(), filename.end());

		if (lodepng::encode(lodepngfname, v, size.w, size.h))
			ensure(0);
	}

	void image::fill(unsigned char val) {
		memset(v.data(), val, get_bytes());
	}

	void image::fill(unsigned char* channel_vals) {
		int i = 0, bytes = get_bytes(), c = 0;

		while (i < bytes)
			for (c = 0; c < channels; ++c)
				v[i++] = channel_vals[c];
	}

	void image::fill_channel(int ch, unsigned char val) {
		int i = 0, bytes = get_bytes() / channels;

		while (i < bytes) v[ch + channels*i++] = val;
	}

	void image::copy(const image& img) {
		create(img.size.w, img.size.h, img.channels);

		memcpy(v.data(), img.v.data(), get_bytes());
		channels = img.channels;
	}

	void image::copy(unsigned char* ptr, int _channels, int pitch, const rects::wh<int>& size) {
		create(size.w, size.h, _channels);
		int wbytes = size.w*_channels;

		for (int i = 0; i < size.h; ++i)
			memcpy(v.data() + wbytes*i, ptr + pitch*i, wbytes);

		channels = _channels;
	}


#define LOOP  for(int s_y = src.y, d_y = y; s_y < src.b(); ++s_y, ++d_y) \
	for(int s_x = src.x, d_x = x; s_x < src.r(); ++s_x, ++d_x)
#define SLOOP for(int s_y = src_rc.y, d_y = dest_rc.y; (s_y < src_rc.b() && d_y < dest_rc.b()); ++s_y, ++d_y) \
	for(int s_x = src_rc.x, d_x = dest_rc.x; (s_x < src_rc.r() && d_x < dest_rc.r()); ++s_x, ++d_x) 
#define BCH(ch, src_ch) *ptr(d_x, d_y, ch) = src.flipped ? (img.pix(s_y, s_x, src_ch)) : (img.pix(s_x, s_y, src_ch))
#define BCH_ADD(ch, src_ch) *ptr(d_x, d_y, ch) += src.flipped ? (img.pix(s_y, s_x, src_ch)) : (img.pix(s_x, s_y, src_ch))
#define DCH(ch) (ptr(d_x, d_y, ch))
#define SCH(src_ch) (src.flipped ? (img.pix(s_y, s_x, src_ch)) : (img.pix(s_x, s_y, src_ch)))


	void image::blit(const image& img, int x, int y, const rects::xywhf<int>& src, bool luminance_to_alpha, bool add) {
		int c;
		if (channels == img.channels) {
			if (!add) {
				LOOP{
					for (c = 0; c < channels; ++c) BCH(c, c);
				}
			}
			else {
				LOOP{
					for (c = 0; c < channels; ++c) BCH_ADD(c, c);
				}
			}
		}
		else {
			if (channels <= 2) {
				if (img.channels == 1) {
					if (luminance_to_alpha) {
						LOOP{
							*DCH(0) = 255;
							BCH(1, 0);
						}
					}
					else LOOP BCH(0, 0);
				}
				else {
					LOOP *DCH(0) = (SCH(0) + SCH(1) + SCH(2)) / 3;

					if (channels == 2 && img.channels == 4) LOOP BCH(1, 3);
				}
			}
			if (channels >= 3) {
				if (img.channels == 2) {
					LOOP{
						BCH(0, 0);
						BCH(1, 0);
						BCH(2, 0);
					}
					if (channels == 4) LOOP BCH(3, 1);
				}
				else if (img.channels == 1) {
					if (channels == 4 && luminance_to_alpha)
						LOOP{
							BCH(3, 0);
							*DCH(0) = *DCH(1) = *DCH(2) = 255;
					}
					else
						LOOP{
							BCH(0, 0);
							BCH(1, 0);
							BCH(2, 0);
					}
				}
				else {
					if (luminance_to_alpha && channels == 4)
						LOOP{
							BCH(0, 0);
							BCH(1, 1);
							BCH(2, 2);
							*DCH(3) = (SCH(0) + SCH(1) + SCH(2)) / 3;
					}
					else
						LOOP{
							BCH(0, 0);
							BCH(1, 1);
							BCH(2, 2);
					}
				}
			}
		}
	}


	void image::blit_channel(const image& img, int x, int y, const rects::xywhf<int>& src, int ch, int src_ch) {
		LOOP BCH(ch, src_ch);
	}

	unsigned char* image::operator()(int x, int y, int channel) {
		return ptr(x, y, channel);
	}

	unsigned char* image::ptr(int x, int y, int channel) {
		return v.data() + (static_cast<int>(size.w) * y + x) * channels + channel;
	}
	
	const unsigned char* image::ptr(int x, int y, int channel) const {
		return v.data() + (static_cast<int>(size.w) * y + x) * channels + channel;
	}

	unsigned char image::pix(int x, int y, int channel) const {
		return v[(static_cast<int>(size.w) * y + x) * channels + channel];
	}

	rgba& image::pixel(int x, int y) {
		return *(rgba*)ptr(x, y, 0);
	}

	const rgba& image::pixel(int x, int y) const {
		return *(rgba*)ptr(x, y, 0);
	}

	const rgba& image::pixel(vec2i p) const {
		return *(rgba*)ptr(p.x, p.y, 0);
	}

	int image::get_bytes() const {
		return sizeof(unsigned char) * static_cast<int>(size.w) * static_cast<int>(size.h) * channels;
	}

	int image::get_channels() const {
		return channels;
	}

	int image::get_num_pixels() const {
		return get_bytes() / get_channels();
	}

	const rects::wh<int>& image::get_size() const {
		return size;
	}

	void image::destroy() {
		v.clear();
		v.shrink_to_fit();
	}
}




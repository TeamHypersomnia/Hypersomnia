#pragma once
#include <Windows.h>
#include <gdiplus.h>
#include <memory>

#include "image.h"
#include "lodepng.h"

#include "ensure.h"

namespace augs {
	using namespace Gdiplus;

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

	void image::paint_circle_midpoint(int radius, augs::rgba filling, bool constrain_angle, float angle_start, float angle_end, bool scale_alpha) {
		auto side = radius * 2 + 1;

		if (v.empty()) {
			create(side, side, 4);
		}
		else {
			ensure(size.w >= side);
			ensure(size.h >= side);
		}

		int x = radius;
		int y = 0;
		int decisionOver2 = 1 - x;   // Decision criterion divided by 2 evaluated at x=r, y=0
		int x0 = size.w / 2;
		int y0 = size.h / 2;
		
		auto pp = [&](int x, int y){
			int x_center = x - size.w / 2;
			int y_center = y - size.h / 2;

			auto angle = vec2(x_center, y_center).degrees();

			if (!constrain_angle || (angle >= angle_start && angle <= angle_end)) {
				auto col = filling;
				if (scale_alpha)
					col.a = (angle - angle_start) / (angle_end - angle_start) * 255;

				pixel(x, y) = col;
			}
		};

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

	bool image::from_file(const std::wstring& filename, bool swap_red_and_blue, unsigned force_channels) {
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

		if(swap_red_and_blue)
			for (int i = 0; i < size.area(); ++i)
				std::swap(v[i * 4 + 0], v[i * 4 + 2]);

		return true;
	}

	void image::save(const std::wstring& filename) {
		std::string lodepngfname(filename.begin(), filename.end());

		if (lodepng::encode(lodepngfname, v, size.w, size.h))
			ensure(0);
	}

	bool image::from_clipboard() {
		using namespace Gdiplus;

		bool ret = false;
		if (OpenClipboard(0)) {
			HBITMAP h = (HBITMAP)GetClipboardData(CF_BITMAP);
			if (h) {
				Bitmap b(h, 0);
				BitmapData bi = { 0 };
				b.LockBits(&Rect(0, 0, b.GetWidth(), b.GetHeight()), ImageLockModeRead, PixelFormat24bppRGB, &bi);
				create(b.GetWidth(), b.GetHeight(), 3);

				for (unsigned i = 0; i < bi.Height; ++i)
					for (unsigned j = 0; j < bi.Width; ++j) {
						*ptr(j, i, 0) = ((unsigned char*)bi.Scan0)[i*bi.Stride + j * 3 + 2];
						*ptr(j, i, 1) = ((unsigned char*)bi.Scan0)[i*bi.Stride + j * 3 + 1];
						*ptr(j, i, 2) = ((unsigned char*)bi.Scan0)[i*bi.Stride + j * 3 + 0];
					}

				DeleteObject(h);
			}
			CloseClipboard();
		}
		return ret;
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
#define DCH(ch) (ptr(d_x, d_y, ch))
#define SCH(src_ch) (src.flipped ? (img.pix(s_y, s_x, src_ch)) : (img.pix(s_x, s_y, src_ch)))


	void image::blit(const image& img, int x, int y, const rects::xywhf<int>& src, bool luminance_to_alpha) {
		int c;
		if (channels == img.channels) {
			LOOP{
				for (c = 0; c < channels; ++c) BCH(c, c);
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




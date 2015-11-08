#pragma once
#include <vector>
#include "../math/vec2.h"
/*
todo:
* ostateczne rozwiazanie kwestii rejestru, podzielic po prostu na link/object ale na razie to nie moj concern
*/

namespace augs {
	namespace texture_baker {
		class image {
			std::vector<unsigned char> v;
			rects::wh<int> size;
			int channels;
			
		public:
			image();
			image(const image&);
			image& operator=(const image&);

			void create	  (int w, int h, int channels);
			bool from_file(const std::wstring& filename, unsigned channels = 0),
				from_clipboard();

			void fill(unsigned char val), 
				fill(unsigned char* channel_vals),
				fill_channel(int channel, unsigned char val);

			void copy(const image&);
			void copy(unsigned char* ptr, int channels, int pitch, const rects::wh<int>& size);

			void blit		 (const image&, int x, int y, const rects::xywhf<int>& src, bool luminance_to_alpha = false);
			void blit_channel(const image&, int x, int y, const rects::xywhf<int>& src, int channel, int src_channel);

			unsigned char* operator()(int x, int y, int channel = 0); // get pixel
			unsigned char* ptr(int x = 0, int y = 0, int channel = 0);
			unsigned char  pix(int x = 0, int y = 0, int channel = 0) const;

			int get_channels() const, get_bytes() const, get_num_pixels() const;
			const rects::wh<int>& get_size() const;

			void destroy();
		};
	}
}
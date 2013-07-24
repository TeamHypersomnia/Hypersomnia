#pragma once
#include <vector>
#include "../math/rects.h"
/*
todo:
* ostateczne rozwiazanie kwestii rejestru, podzielic po prostu na link/object ale na razie to nie moj concern
*/

namespace augmentations {
	namespace texture_baker {
		class image {
			std::vector<unsigned char> v;
			rects::wh size;
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
			void copy(unsigned char* ptr, int channels, int pitch, const rects::wh& size);

			void blit		 (const image&, int x, int y, const rects::xywhf& src, bool luminance_to_alpha = false);
			void blit_channel(const image&, int x, int y, const rects::xywhf& src, int channel, int src_channel);

			unsigned char* operator()(int x, int y, int channel = 0); // get pixel
			unsigned char* ptr(int x = 0, int y = 0, int channel = 0);
			unsigned char  pix(int x = 0, int y = 0, int channel = 0) const;

			int get_channels() const, get_bytes() const, get_num_pixels() const;
			const rects::wh& get_size() const;

			void destroy();
		};

		//class image_file : public image, public augmentations::util::container::registry<image_file> {
		//	const char* filename;
		//	int force_channels;
		//public:
		//	bool opened, operator==(const image_file&);
		//	image_file&  operator= (const image_file&); // set

		//	image_file();
		//	void open(const char* filename, unsigned force_channels = 0), destroy();
		//	bool is_open();
		//	
		//	~image_file();
		//};
	}
}
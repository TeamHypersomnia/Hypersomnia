#pragma once
#include "stdafx.h"
#include <memory>

#include "image.h"

namespace augs {
	namespace texture_baker {
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
			size = vec2<int>(w, h);
			channels = ch;
			v.resize(get_bytes());
			//v.shrink_to_fit();
		}

		bool image::from_file(const std::wstring& filename, unsigned force_channels) {
			std::string errstr("Coudn't load ");
			errstr += std::string(filename.begin(), filename.end());

			channels = force_channels;
			std::shared_ptr<Gdiplus::Bitmap> pBitmap(Gdiplus::Bitmap::FromFile(filename.c_str(), FALSE));

			if (pBitmap.get() == 0 || pBitmap.get()->GetLastStatus() != Gdiplus::Ok) 
				throw std::runtime_error(errstr.c_str());

			// GDI+ orients bitmap images top-bottom.
			// OpenGL expects bitmap images to be oriented bottom-top by default.
			//pBitmap->RotateFlip(Gdiplus::RotateNoneFlipY);

			// GDI+ pads each scanline of the loaded bitmap image to 4-byte memory
			// boundaries. Fortunately OpenGL also aligns bitmap images to 4-byte
			// memory boundaries by default.
			int width = pBitmap->GetWidth();
			int height = pBitmap->GetHeight();
			//int pitch = ((width * 32 + 31) & ~31) >> 3;

			int format = PixelFormat32bppARGB;

			switch(channels) {
			case 0: format = pBitmap->GetPixelFormat(); 
				switch(format) {
				case PixelFormat8bppIndexed: 
					channels = 1; break;
				case PixelFormat16bppGrayScale: 
					channels = 2; break;
				case PixelFormat24bppRGB: 
					channels = 3; break;
				case PixelFormat32bppARGB: 
					channels = 4; break;
				default: throw std::runtime_error(errstr.c_str()); break;
				}
				break;
			case 1: format = PixelFormat8bppIndexed; break;
			case 2: format = PixelFormat16bppGrayScale; break;
			case 3: format = PixelFormat24bppRGB; break;
			case 4: format = PixelFormat32bppARGB; break;
			default: format = PixelFormat32bppARGB; break;
			}

			create(width, height, channels);
			//v.resize(pitch * height);
			//size = rects::wh(width, height);
			Gdiplus::BitmapData bdata;
			Gdiplus::Rect rect(0, 0, width, height);

			// Convert to 32-bit BGRA pixel format and fetch the pixel data.
			if (pBitmap->LockBits(&rect, Gdiplus::ImageLockModeRead, format, &bdata) != Gdiplus::Ok)
				return false;

			if (bdata.Stride == width*channels)
				memcpy(&v[0], bdata.Scan0, width * height * channels);
			else {
				unsigned char *pSrcPixels = static_cast<unsigned char *>(bdata.Scan0);

				for (int i = 0; i < height; ++i)
					memcpy(&v[i * width * channels], &pSrcPixels[i * bdata.Stride], width * channels);
			}

			pBitmap->UnlockBits(&bdata);

			return true;
		}

		bool image::from_clipboard() {
			using namespace Gdiplus;

			bool ret = false;
			if(OpenClipboard(0)) {
				HBITMAP h = (HBITMAP)GetClipboardData(CF_BITMAP);
				if(h) {
					Bitmap b(h, 0);
					BitmapData bi = {0};
					b.LockBits(&Rect(0,0,b.GetWidth(), b.GetHeight()), ImageLockModeRead, PixelFormat24bppRGB, &bi);
					create(b.GetWidth(), b.GetHeight(), 3);

					for(unsigned i = 0; i < bi.Height; ++i)
						for(unsigned j = 0; j < bi.Width; ++j) {
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

			while(i < bytes)
				for(c = 0; c < channels; ++c)
					v[i++] = channel_vals[c];
		}

		void image::fill_channel(int ch, unsigned char val) {
			int i = 0, bytes = get_bytes()/channels;

			while(i < bytes) v[ch+channels*i++] = val;
		}

		void image::copy(const image& img) {
			create(img.size.w, img.size.h, img.channels);

			memcpy(v.data(), img.v.data(), get_bytes());
			channels = img.channels;
		}

		void image::copy(unsigned char* ptr, int _channels, int pitch, const rects::wh<int>& size) {
			create(size.w, size.h, _channels);
			int wbytes = size.w*_channels;

			for(int i = 0; i < size.h; ++i)
				memcpy(v.data()+wbytes*i, ptr+pitch*i, wbytes);

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
			if(channels == img.channels) {										   
				LOOP {
					for(c = 0; c < channels; ++c) BCH(c, c);												   
				}
			}
			else {
				if(channels <= 2) {
					if(img.channels == 1) {	
						if(luminance_to_alpha) {
							LOOP {
								*DCH(0) = 255;	
								BCH(1, 0);
							}
						}
						else LOOP BCH(0, 0);
					}
					else {						   
						LOOP *DCH(0) = (SCH(0) + SCH(1) + SCH(2)) / 3;															   

						if(channels == 2 && img.channels == 4) LOOP BCH(1, 3);
					}
				}
				if(channels >= 3) {                                                            
					if(img.channels == 2) {													   
						LOOP {																   
							BCH(0, 0);														   
							BCH(1, 0);														   
							BCH(2, 0);														   
						}																	   
						if(channels == 4) LOOP BCH(3, 1);			   			   
					}
					else if(img.channels == 1) {
						if(channels == 4 && luminance_to_alpha) 
							LOOP {
								BCH(3, 0);
								*DCH(0) = *DCH(1) = *DCH(2) = 255;
						}
						else										   
							LOOP {																   
								BCH(0, 0);														   
								BCH(1, 0);														   
								BCH(2, 0);														   
						}						
					}
					else {
						if(luminance_to_alpha && channels == 4)
							LOOP {																   
								BCH(0, 0);														   
								BCH(1, 1);														   
								BCH(2, 2);			
								*DCH(3) = (SCH(0) + SCH(1) + SCH(2))/3;
						}	
						else
							LOOP {																   
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

		unsigned char image::pix(int x, int y, int channel) const {
			return v[(static_cast<int>(size.w) * y + x) * channels + channel];
		}


		int image::get_bytes() const {
			return sizeof(unsigned char) * static_cast<int>(size.w) * static_cast<int>(size.h) * channels;
		}

		int image::get_channels() const {
			return channels;
		}

		int image::get_num_pixels() const {
			return get_bytes()/get_channels();
		}

		const rects::wh<int>& image::get_size() const {
			return size;
		}

		void image::destroy() {
			v.clear();
			v.shrink_to_fit();
		}
		/*

		bool image_file::operator==(const image_file& b) {
		return (!strcmp(filename, b.filename) && (force_channels == b.force_channels || force_channels == b.get_channels()));
		}

		image_file& image_file::operator=(const image_file& b) {
		set(b);
		filename = b.filename; force_channels = b.force_channels; opened = true;
		return *this;
		}

		image_file::image_file() : opened(false) {}

		void image_file::open(const char* _filename, unsigned _force_channels) {
		destroy();
		filename = _filename; force_channels = _force_channels;

		if(assign()) {
		data = SOIL_load_image(filename, &size.w, &size.h, &channels, force_channels);
		if(channels == force_channels) force_channels = 0;
		if(force_channels) channels = force_channels;
		}

		opened = true;
		}

		bool image_file::is_open() {
		return opened;
		}

		void image_file::destroy() {
		if(opened)
		if(deassign()) {
		SOIL_free_image_data(data);
		opened = false;
		}
		}

		image_file::~image_file() {
		destroy();
		}*/
	}
}




#pragma once
#include "graphics/pixel.h"
#include "texture_baker/texture_baker.h"

namespace augs {
	namespace graphics {
		namespace gui {
			extern augs::texture_baker::texture* null_texture;

			struct material {
				augs::texture_baker::texture* tex;
				pixel_32 color;
				material(augs::texture_baker::texture* = null_texture, const pixel_32& = pixel_32()); 
				material(const pixel_32&); 
			};

			struct vertex {
				float x, y;
				float u, v;
				pixel_32 col;
			};

			struct quad {
				vertex p[4];
				quad();
				quad(const rect_ltrb&, const material&, const rect_texture& = rect_texture(0.f, 0.f, 1.f, 1.f));

				static quad clipped(const rect_ltrb&, const rect_ltrb&, const material&);
				void clip(const rect_ltrb&);
				void move(const point&);
				void set (const rect_ltrb&);
				void rotate90(int times);
				rect_ltrb get_rect() const;
			};

			/* 
			clips and pushes origin to quad vector
			clipper = 0 means no clipping
				returns clipped rectangle
			*/
			extern rect_ltrb add_quad(const material&, const rect_ltrb& origin, const rect_ltrb* clipper, std::vector<quad>& v);
			extern void scale_virtual_res(rect_wh vres, rect_wh display, std::vector<quad>& quads);
		}
	}
}
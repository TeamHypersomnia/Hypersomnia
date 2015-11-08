#pragma once
#include "graphics/pixel.h"
#include "texture_baker/texture_baker.h"
#include "graphics/vertex.h"

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

			//struct vertex {
			//	float x, y;
			//	float u, v;
			//	pixel_32 col;
			//};
			//
			//struct quad {
			//	vertex p[4];
			//	quad();
			//	quad(const rects::ltrb<float>&, const material&, const rects::texture<float>& = rects::texture<float>(0.f, 0.f, 1.f, 1.f));
			//
			//	static quad clipped(const rects::ltrb<float>&, const rects::ltrb<float>&, const material&);
			//	void clip(const rects::ltrb<float>&);
			//	void move(const vec2i&);
			//	void set (const rects::ltrb<float>&);
			//	void rotate90(int times);
			//	rects::ltrb<float> get_rect() const;
			//};

			/* 
			clips and pushes origin to quad vector
			clipper = 0 means no clipping
				returns clipped rectangle
			*/
			extern rects::ltrb<float> add_quad(const material&, const rects::ltrb<float>& origin, const rects::ltrb<float>* clipper, std::vector<resources::vertex_triangle>& v);
			//extern void scale_virtual_res(rects::wh<float> vres, rects::wh<float> display, std::vector<resources::vertex_triangle>& quads);
		}
	}
}
#pragma once
#include <vector>
#include "image.h"
#include "rectpack.h"

namespace augs {
	class texture {
		friend class atlas;
		image* img;
		rects::xywhf<int> rect;
		float x, y, w, h;
		bool ltoa;

		void set_uv_unit(double, double);
	public:

		texture();
		texture(image* img);
		void set(image* img);
		void luminosity_to_alpha(bool);

		rects::xywhf<int> get_rect() const;
		vec2i get_size() const;

		void translate_uv(vec2 pixels);
		void scale_uv(float u_scalar, float v_scalar);
		void get_uv(const rects::texture<float>& uv, rects::texture<float>& out) const;
		void get_uv(float u, float v, float& u_out, float& v_out) const;
		void get_uv(vec2& texture_space) const;

		/* gets u coordinate from a standard rectangular quad with origin coordinates 0.0, 0.0, 1.0, 1.0 */
		float get_u(int vertex_num_from_cw_rect) const;
		/* gets v coordinate from a standard rectangular quad with origin coordinates 0.0, 0.0, 1.0, 1.0 */
		float get_v(int vertex_num_from_cw_rect) const;

		float get_u_unit() const, get_v_unit() const;
	};

	class atlas {
		static unsigned current;
		unsigned id;
		bool mipmaps, lin, rep, built;
		std::vector<rects::xywhf<int>*> ptr_arr; // for reallocations
		packing::bin b;
		float adder, mult;

	public:
		atlas(int white_pixel_size = 0); ~atlas();

		std::vector<texture*> textures;

		texture atlas_texture;
		image img;

		bool pack(), // max texture size 
			pack(int max_size);

		void create_image(int atlas_channels, bool destroy_images);
		void build(bool mipmaps = false, bool linear = false, image* raw_texture = 0), bind(), _bind(), nearest(), linear(), clamp(), repeat();

		void default_build();

		bool is_mipmapped() const;
		void destroy();
	};
}
#pragma once
#include <vector>
#include "augs/math/vec2.h"
#include "augs/graphics/pixel.h"

namespace augs {
	class image {
		std::vector<rgba> v;
		vec2u size;

	public:
		void create(const vec2u image_size);

		template <class A, class B>
		void create(const A columns, const B rows) {
			create({ columns, rows });
		}

		void create_from(
			const rgba_channel* const ptr,
			const unsigned channels,
			const unsigned pitch,
			const vec2u size
		);
		
		void fill(const rgba fill_color);

		bool from_file(const std::string& filename);
		bool from_clipboard();

		void blit(
			const image& source,
			const vec2u destination,
			const bool flip_source = false,
			const bool add_rgba_values = false
		);

		void paint_circle(
			const unsigned radius, 
			const unsigned border_width = 1, 
			const rgba filling = white, 
			const bool scale_alpha = false
		);
		
		void paint_circle_midpoint(
			const unsigned radius, 
			const unsigned border_width = 1, 
			const rgba filling = white, 
			const bool scale_alpha = false, 
			const bool constrain_angle = false,
			const vec2 angle_start = vec2(), 
			const vec2 angle_end = vec2()
		);

		void paint_filled_circle(
			const unsigned radius, 
			const rgba filling = white
		);

		void paint_line(
			const vec2u from, 
			const vec2u to, 
			const rgba filling = white
		);
		
		void swap_red_and_blue();

		rgba& pixel(const vec2u at_coordinates);

		void destroy();
		
		void save(const std::string& filename) const;
		
		vec2u get_size() const;
		unsigned get_rows() const;
		unsigned get_columns() const;

		bool in_bounds(const vec2u at_coordinates) const;
		std::vector<vec2i> get_polygonized() const;
		const rgba_channel* get_data() const;
		const rgba& pixel(const vec2u at_coordinates) const;

		image get_desaturated() const;
	};
}
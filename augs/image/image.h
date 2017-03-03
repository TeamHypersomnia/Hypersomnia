#pragma once
#include <vector>
#include "augs/math/vec2.h"
#include "augs/graphics/pixel.h"
#include "augs/padding_byte.h"
#include "augs/misc/trivial_variant.h"
#include "augs/templates/maybe_const.h"

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

		struct paint_circle_midpoint_command {
			unsigned radius;
			unsigned border_width = 1;
			bool scale_alpha = false;
			bool constrain_angle = false;
			padding_byte pad[2];
			float angle_start = 0.f;
			float angle_end = 0.f;
			rgba filling = white;

			static std::string get_command_name() {
				return "circle_midpoint";
			}
		};
		
		struct paint_circle_filled_command {
			unsigned radius;
			rgba filling = white;

			static std::string get_command_name() {
				return "circle_filled";
			}
		};

		struct paint_line_command {
			vec2u from;
			vec2u to;
			rgba filling;

			static std::string get_command_name() {
				return "line";
			}
		};

		typedef augs::trivial_variant<
			paint_circle_midpoint_command,
			paint_circle_filled_command,
			paint_line_command
		> command_variant;

		void execute(const command_variant&);

		void execute(const paint_circle_midpoint_command&);
		void execute(const paint_circle_filled_command&);
		void execute(const paint_line_command&);
		
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

	template <bool C, class F>
	auto introspect(
		maybe_const_ref_t<C, image::paint_circle_midpoint_command> t,
		F f
	) {
		f(t.radius);
		f(t.border_width);
		f(t.scale_alpha);
		f(t.constrain_angle);
		f(t.angle_start);
		f(t.angle_end);
		f(t.filling);
	}

	template <bool C, class F>
	auto introspect(
		maybe_const_ref_t<C, image::paint_circle_filled_command> t,
		F f
	) {
		f(t.radius);
		f(t.filling);
	}

	template <bool C, class F>
	auto introspect(
		maybe_const_ref_t<C, image::paint_line_command> t,
		F f
	) {
		f(t.from);
		f(t.to);
		f(t.filling);
	}
}

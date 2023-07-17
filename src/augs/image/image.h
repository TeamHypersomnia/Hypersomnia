#pragma once
#include <vector>
#include <variant>
#include <memory>

#include "augs/pad_bytes.h"
#include "augs/templates/exception_templates.h"

#include "augs/math/vec2.h"
#include "augs/graphics/rgba.h"

#include "augs/filesystem/path.h"

namespace augs {
	struct image_loading_error : error_with_typesafe_sprintf {
		using error_with_typesafe_sprintf::error_with_typesafe_sprintf;
	};

	struct paint_circle_midpoint_command {
		// GEN INTROSPECTOR struct augs::paint_circle_midpoint_command
		unsigned radius = 0u;
		unsigned border_width = 1;
		bool scale_alpha = false;
		bool constrain_angle = false;
		pad_bytes<2> pad;
		float angle_start = 0.f;
		float angle_end = 0.f;
		rgba filling = white;
		// END GEN INTROSPECTOR

		static const char* get_custom_type_name() {
			return "circle_midpoint";
		}
	};

	struct paint_circle_filled_command {
		// GEN INTROSPECTOR struct augs::paint_circle_filled_command
		unsigned radius = 0u;
		rgba filling = white;
		// END GEN INTROSPECTOR

		static const char* get_custom_type_name() {
			return "circle_filled";
		}
	};

	struct paint_line_command {
		// GEN INTROSPECTOR struct augs::paint_line_command
		vec2u from;
		vec2u to;
		rgba filling = white;
		// END GEN INTROSPECTOR

		static const char* get_custom_type_name() {
			return "line";
		}
	};

	using paint_command_variant = std::variant<
		paint_circle_midpoint_command,
		paint_circle_filled_command,
		paint_line_command
	>;

	class image {
		std::vector<rgba> v;
		vec2u size;

		void load_stbi_buffer(unsigned char*, int w, int h);
	public:

		enum class scaling_method {
			STB
		};

		struct frame {
			std::vector<std::byte> serialized_frame;
			float duration_milliseconds = 0.0f;
		};

		using gif_data = std::vector<frame>;

		static gif_data gif_to_frames(const path_type& file_path);
		static std::vector<int> read_gif_frame_meta(const path_type& file_path);

		static image white_pixel();

		static vec2u get_size(const path_type& file_path);
		static vec2u get_size(const std::vector<std::byte>& bytes);
		static vec2u get_png_size(const std::vector<std::byte>& bytes);

		void from_file(const path_type& path);
		void from_png(const path_type& path);
		void from_binary_file(const path_type& path);

		void from_bytes(
			const std::vector<std::byte>& from, 
			const path_type& reported_path
		);

		void from_image_bytes(
			const std::vector<std::byte>& from, 
			const path_type& reported_path
		);

		image(const vec2u image_size = {});
		
		image(
			const rgba_channel* const ptr,
			const vec2u size,
			const unsigned channels = 4,
			const unsigned pitch = 0
		);

		image(std::vector<rgba>&& in, vec2u size) : v(std::move(in)), size(size) {}
		
		void fill(const rgba fill_color);
		void flip_y();

		void execute(const paint_command_variant&);

		void execute(const paint_circle_midpoint_command&);
		void execute(const paint_circle_filled_command&);
		void execute(const paint_line_command&);
		
		void swap_red_and_blue();

		void resize_no_fill(const vec2u new_size) {
			size = new_size;
			v.resize(new_size.area());
		}

		void scale(vec2u new_size, scaling_method = scaling_method::STB);

		void resize_fill(const vec2u new_size, const rgba col = rgba(0, 0, 0, 0)) {
			size = new_size;
			v.resize(new_size.area(), col);
		}

		void clear() {
			v.clear();
			size = {};
		}

		std::vector<std::byte> to_image_bytes() const;
		void save_as_png(const path_type& path) const;
		void save_as_binary_file(const path_type& path) const;

		void save(const augs::path_type& path) const;
		
		/* 
			These are performance-critical functions,
			so let them be inlined even in builds without link-time optimization enabled.
		*/

		rgba_channel* data() {
			return std::addressof(v.data()->r);
		}

		const rgba_channel* data() const {
			return std::addressof(v.data()->r);
		}

		vec2u get_size() const {
			return size;
		}

		unsigned get_rows() const {
			return size.y;
		}

		unsigned get_columns() const {
			return size.x;
		}

		rgba& pixel(const unsigned pos) {
			return v[pos];
		}

		const rgba& pixel(const unsigned pos) const {
			return v[pos];
		}

		rgba& pixel(const vec2u pos) {
			return v[pos.y * size.x + pos.x];
		}

		const rgba& pixel(const vec2u pos) const {
			return v[pos.y * size.x + pos.x];
		}

		auto to_pixel_coord(const unsigned pos) const {
			const auto y = pos / size.x;
			const auto x = pos - y * size.x;

			return vec2u(x, y);
		}

		bool in_bounds(const vec2u p) const {
			return p.x < size.x && p.y < size.y;
		}

		image& desaturate();

		auto begin() {
			return v.begin();
		}

		auto begin() const {
			return v.begin();
		}

		auto end() {
			return v.end();
		}

		auto end() const {
			return v.end();
		}
	};

	class image_view {
		rgba* const v;
		const vec2u size;

	public:
		image_view(rgba* v, vec2u size);

		rgba& pixel(const vec2u pos) {
			return v[pos.y * size.x + pos.x];
		}

		const rgba& pixel(const vec2u pos) const {
			return v[pos.y * size.x + pos.x];
		}

		void fill(const rgba fill_color);

		auto* data() {
			return std::addressof(v->r);
		}

		const auto* data() const {
			return std::addressof(v->r);
		}

		auto get_size() const {
			return size;
		}
	};
}
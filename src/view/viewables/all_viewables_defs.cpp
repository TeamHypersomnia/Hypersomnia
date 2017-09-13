#include <Box2D/Box2D.h>

#include "augs/templates/introspect.h"

#include "game/assets/all_logical_assets.h"
#include "view/viewables/all_viewables_defs.h"

void all_viewables_defs::update_into(all_logical_assets& output) {
	const auto sound_duration = [](const augs::sound_buffer::variation& v) {
		return v.stereo_or_mono().get_length_in_seconds();
	};

	for (const auto& s : sounds) {
		const auto& source = s.second;
		auto& logical = output[s.first];
		const auto buffer = augs::sound_buffer(source);

		logical.num_of_variations = buffer.variations.size();

		logical.max_duration_in_seconds = static_cast<float>(sound_duration(maximum_of(
			buffer.variations,
			[sound_duration](const auto& a, const auto& b) {
				return sound_duration(a) < sound_duration(b);
			}
		)));
	}

	for (const auto& s : particle_effects) {
		const auto& source = s.second;
		auto& logical = output[s.first];

		logical.max_duration_in_seconds = maximum_of(
			source.emissions,
			[](const particles_emission& a, const particles_emission& b) {
				return a.stream_lifetime_ms.second < b.stream_lifetime_ms.second;
			}
		).stream_lifetime_ms.second;
	}

	for (const auto& s : game_image_loadables) {
		const auto& source = s.second;
		auto& logical = output[s.first];

		logical.original_image_size = source.get_size();
	}

	for (const auto& s : game_image_metas) {
		const auto& source = s.second;
		auto& logical = output[s.first];
		
		auto& shape = logical.shape;

		if (source.physical_shape.has_value()) {
			const auto image_size = vec2(logical.original_image_size);

			std::vector<vec2> new_concave;

			for (vec2 v : source.physical_shape.value()) {
				v.y = -v.y;
				new_concave.push_back(v);
			}

			const auto origin = image_size / vec2(-2, 2);

			for (auto& v : new_concave) {
				v += origin;
			}

			shape.add_concave_polygon(new_concave);
			shape.scale(vec2(1, -1));

			for (auto& c : shape.convex_polys) {
				reverse_container(c);
			}
		}
		else {
			const auto box_size = vec2(logical.original_image_size) / 2;
			
			// TODO: use ltrb

			b2PolygonShape poly_shape;
			poly_shape.SetAsBox(box_size.x, box_size.y);

			convex_partitioned_shape::convex_poly new_convex_polygon;

			for (int i = 0; i < poly_shape.GetVertexCount(); ++i) {
				new_convex_polygon.push_back(vec2(poly_shape.GetVertex(i)));
			}

			shape.add_convex_polygon(new_convex_polygon);
		}
	}
}
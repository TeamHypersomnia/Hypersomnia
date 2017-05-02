#include "tile_layer.h"

#include "augs/templates/container_templates.h"

#include "game/assets/assets_manager.h"

tile_layer::tile::tile(const unsigned type) : type_id(type) {}
//tile_layer::tile_type::tile_type(const tile_type_id tile_texture) : tile_texture(tile_texture) {}

vec2u tile_layer::get_size() const {
	return size;
}

tile_layer::tile& tile_layer::tile_at(const vec2u p) {
	return tiles[p.y * size.x + p.x];
}

const tile_layer::tile& tile_layer::tile_at(const vec2u p) const {
	return tiles[p.y * size.x + p.x];
}

tile_layer::tile_type& tile_layer::get_tile_type(const tile_type_id t) {
	return tileset[t - 1];
}

const tile_layer::tile_type& tile_layer::get_tile_type(const tile_type_id t) const {
	return tileset[t - 1];
}

tile_layer::tile_type& tile_layer::get_tile_type(const tile& t) {
	return tileset[t.type_id - 1];
}

const tile_layer::tile_type& tile_layer::get_tile_type(const tile& t) const {
	return tileset[t.type_id - 1];
}

void tile_layer::expand(const vec2u new_area) {
	std::vector<tile> new_tiles;
	new_tiles.resize(new_area.area());

	for (size_t x = 0; x < size.x; ++x) {
		for (size_t y = 0; y < size.y; ++y) {
			new_tiles[y * new_area.x + x] = tiles[y * size.x + x];
		}
	}

	tiles = std::move(new_tiles);
	size = new_area;
}

void tile_layer::expand_to_position(const vec2u pos) {
	expand({ std::max(pos.x, size.x), std::max(pos.y, size.y) });
}

void tile_layer::expand_to_rect(const ltrbu rc) {
	expand_to_position(rc.right_bottom());
}

void tile_layer::set_tile(const vec2u pos, const tile_type_id id) {
	expand_to_position(pos);

	tile_at(pos).type_id = id;
}

void tile_layer::set_tiles(const ltrbu rc, const tile_type_id id) {
	expand_to_rect(rc);
	const auto id_index = id;

	for (size_t x = rc.l; x < rc.r; ++x) {
		for (size_t y = rc.t; y < rc.b; ++y) {
			tile_at({ x, y }).type_id = id_index;
		}
	}
}

void tile_layer::set_tiles_alternating(const ltrbu rc, const tile_type_id id_1, const tile_type_id id_2) {
	expand_to_rect(rc);
	const auto id_1_index = id_1;
	const auto id_2_index = id_2;

	for (size_t x = rc.l; x < rc.r; ++x) {
		if ((x - rc.l) % 2 == 0) {
			for (size_t y = rc.t; y < rc.b; ++y) {
				tile_at({ x, y }).type_id = (y - rc.t) % 2 == 0 ? id_1_index : id_2_index;
			}
		}
		else {
			for (size_t y = rc.t; y < rc.b; ++y) {
				tile_at({ x, y }).type_id = (y - rc.t) % 2 == 1 ? id_1_index : id_2_index;
			}
		}
	}
}

void tile_layer::set_tiles(const ltrbu rc, const tile_rectangular_filling id) {
	ensure(rc.w() > 2 && rc.h() > 2)

		expand_to_rect(rc);

	const unsigned fill = id.fill;

	const unsigned left_border = (id.left_border);
	const unsigned top_border = (id.top_border);
	const unsigned right_border = (id.right_border);
	const unsigned bottom_border = (id.bottom_border);

	const unsigned lt_corner = (id.lt_corner);
	const unsigned rt_corner = (id.rt_corner);
	const unsigned rb_corner = (id.rb_corner);
	const unsigned lb_corner = (id.lb_corner);

	tile_at(rc.left_top()).type_id = lt_corner;
	tile_at(rc.right_top() - vec2u(1, 0)).type_id = rt_corner;
	tile_at(rc.right_bottom() - vec2u(1, 1)).type_id = rb_corner;
	tile_at(rc.left_bottom() - vec2u(0, 1)).type_id = lb_corner;

	for (size_t x = rc.l + 1; x < rc.r - 1; ++x) {
		tile_at({ x, rc.t }).type_id = top_border;
		tile_at({ x, rc.b - 1 }).type_id = bottom_border;
	}

	for (size_t y = rc.t + 1; y < rc.b - 1; ++y) {
		tile_at({ rc.l, y }).type_id = left_border;
		tile_at({ rc.r - 1, y }).type_id = right_border;
	}

	for (size_t x = rc.l + 1; x < rc.r - 1; ++x) {
		for (size_t y = rc.t + 1; y < rc.b - 1; ++y) {
			tile_at({ x, y }).type_id = fill;
		}
	}
}

unsigned tile_layer::register_tile_type(
	const assets_manager& manager,
	const tile_type new_type
) {
	tileset.push_back(new_type);

	const auto new_size = new_type.get_size(manager);

	const bool is_square = new_size.x == new_size.y;

	ensure(is_square);

	if (tileset.size() > 1) {
		const bool is_same_size = new_size == tileset[tileset.size() - 2].get_size(manager);

		ensure(is_same_size);
	}

	return tileset.size();
}

tile_layer::visible_tiles_by_type tile_layer::get_visible_tiles_by_type(const ltrbu visible_tiles) const {
	tile_layer::visible_tiles_by_type result;
	auto& indices_by_type = result.tiles_by_type;

	for (unsigned y = visible_tiles.t; y < visible_tiles.b; ++y) {
		for (unsigned x = visible_tiles.l; x < visible_tiles.r; ++x) {
			const auto i = y * size.x + x;
			const auto type = tiles[i].type_id;

			if (type != 0) {
				if (indices_by_type.size() < type + 1) {
					indices_by_type.resize(type + 1);
				}

				indices_by_type[type].push_back(vec2u(x, y));
			}
		}
	}

	return result;
}

tile_layer_logical_meta tile_layer::get_logical_meta(const assets_manager& manager) const {
	return {
		get_tile_side(manager),
		get_size()
	};
}
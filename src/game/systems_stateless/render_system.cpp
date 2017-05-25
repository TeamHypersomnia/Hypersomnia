#include "game/components/render_component.h"
#include "render_system.h"

bool render_system::render_order_compare(const const_entity_handle a, const const_entity_handle b) {
	const auto layer_a = a.get<components::render>().layer;
	const auto layer_b = b.get<components::render>().layer;

	return (layer_a == layer_b && layer_a == render_layer::CAR_INTERIOR) ? are_connected_by_friction(a, b) : layer_a < layer_b;
}
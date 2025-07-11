#pragma once
#include <cstddef>
#include "augs/misc/timing/delta.h"
#include "augs/misc/bound.h"

#include "augs/graphics/vertex.h"

#include "view/viewables/all_viewables_declaration.h"
#include "game/cosmos/entity_id.h"
#include "game/components/transform_component.h"
#include "game/messages/pure_color_highlight_message.h"
#include "view/view_container_sizes.h"

class cosmos;
class interpolation_system;

namespace augs {
	struct drawer;
}


struct draw_renderable_input;

class pure_color_highlight_system {
public:
	struct highlight {
		pure_color_highlight_input in;
		
		double time_of_occurence_seconds = 0.0;
	};

private:
	double global_time_seconds = 0.0;
	std::unordered_map<entity_id, highlight> highlights;
public:
	
	void add(entity_id, pure_color_highlight_input);
	void advance(const augs::delta dt);
	
	void draw_highlights(
		const cosmos& cosm,
		const draw_renderable_input&
	) const;

	void reserve_caches_for_entities(const size_t) const {}
	void clear();
};